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

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86internal_p.h"
  #include "../x86/x86inst.h"
#endif // ASMJIT_BUILD_X86

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminternal_p.h"
  #include "../arm/arminst.h"
#endif // ASMJIT_BUILD_ARM

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::CodeEmitter - Construction / Destruction]
// ============================================================================

CodeEmitter::CodeEmitter(uint32_t type) noexcept
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
    _globalInstOptions(Inst::kOptionReserved),
    _extraReg(),
    _inlineComment(nullptr) {}

CodeEmitter::~CodeEmitter() noexcept {
  if (_code) {
    _addFlags(kFlagDestroyed);
    _code->detach(this);
  }
}

// ============================================================================
// [asmjit::CodeEmitter - Code-Generation]
// ============================================================================

Error CodeEmitter::_emitOpArray(uint32_t instId, const Operand_* operands, size_t count) {
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
// [asmjit::CodeEmitter - Finalize]
// ============================================================================

Label CodeEmitter::getLabelByName(const char* name, size_t nameLength, uint32_t parentId) noexcept {
  return Label(_code ? _code->getLabelIdByName(name, nameLength, parentId) : uint32_t(0));
}

// ============================================================================
// [asmjit::CodeEmitter - Finalize]
// ============================================================================

Error CodeEmitter::finalize() {
  // Does nothing by default, overridden by `CodeBuilder` and `CodeCompiler`.
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeEmitter - Error Handling]
// ============================================================================

Error CodeEmitter::reportError(Error err, const char* message) {
  ErrorHandler* handler = getErrorHandler();
  if (!handler) {
    CodeHolder* code = getCode();
    if (code)
      handler = getCode()->getErrorHandler();
  }

  if (handler) {
    if (!message)
      message = DebugUtils::errorAsString(err);
    handler->handleError(err, message, this);
  }

  return err;
}

// ============================================================================
// [asmjit::CodeEmitter - Label Management]
// ============================================================================

bool CodeEmitter::isLabelValid(uint32_t id) const noexcept {
  uint32_t index = Operand::unpackId(id);
  return _code && index < _code->getLabelCount();
}

// ============================================================================
// [asmjit::CodeEmitter - Emit (High-Level)]
// ============================================================================

ASMJIT_FAVOR_SIZE Error CodeEmitter::emitProlog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (getArchInfo().isX86Family())
    return X86Internal::emitProlog(as<X86Emitter>(), frame);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (getArchInfo().isArmFamily())
    return ArmInternal::emitProlog(as<ArmEmitter>(), frame);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error CodeEmitter::emitEpilog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (getArchInfo().isX86Family())
    return X86Internal::emitEpilog(as<X86Emitter>(), frame);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (getArchInfo().isArmFamily())
    return ArmInternal::emitEpilog(as<ArmEmitter>(), frame);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error CodeEmitter::emitArgsAssignment(const FuncFrame& frame, const FuncArgsAssignment& args) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (getArchInfo().isX86Family())
    return X86Internal::emitArgsAssignment(as<X86Emitter>(), frame, args);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (getArchInfo().isArmFamily())
    return ArmInternal::emitArgsAssignment(as<ArmEmitter>(), frame, args);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

// ============================================================================
// [asmjit::CodeEmitter - Comment]
// ============================================================================

Error CodeEmitter::commentf(const char* fmt, ...) {
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

  return comment(sb.getData(), sb.getLength());
  #else
  ASMJIT_UNUSED(fmt);
  return kErrorOk;
  #endif
}

Error CodeEmitter::commentv(const char* fmt, std::va_list ap) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  StringBuilderTmp<1024> sb;

  Error err = sb.appendFormatVA(fmt, ap);
  if (ASMJIT_UNLIKELY(err))
    return err;

  return comment(sb.getData(), sb.getLength());
  #else
  ASMJIT_UNUSED(fmt);
  ASMJIT_UNUSED(ap);
  return kErrorOk;
  #endif
}

// ============================================================================
// [asmjit::CodeEmitter - Events]
// ============================================================================

Error CodeEmitter::onAttach(CodeHolder* code) noexcept {
  _code = code;
  _codeInfo = code->getCodeInfo();
  _emitterOptions = code->getEmitterOptions();

  onUpdateGlobalInstOptions();
  return kErrorOk;
}

Error CodeEmitter::onDetach(CodeHolder* code) noexcept {
  ASMJIT_UNUSED(code);

  _flags = 0;
  _errorHandler = nullptr;

  _codeInfo.reset();
  _gpRegInfo.reset();

  _emitterOptions = 0;
  _privateData = 0;

  _instOptions = 0;
  _globalInstOptions = Inst::kOptionReserved;
  _extraReg.reset();
  _inlineComment = nullptr;

  return kErrorOk;
}

void CodeEmitter::onUpdateGlobalInstOptions() noexcept {
  const uint32_t kCriticalEmitterOptions = kOptionLoggingEnabled   |
                                           kOptionStrictValidation ;

  _globalInstOptions &= ~Inst::kOptionReserved;
  if ((_emitterOptions & kCriticalEmitterOptions) != 0)
    _globalInstOptions |= Inst::kOptionReserved;
}

ASMJIT_END_NAMESPACE
