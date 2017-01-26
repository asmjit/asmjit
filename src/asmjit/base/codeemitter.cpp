// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../base/assembler.h"
#include "../base/utils.h"
#include "../base/vmem.h"

#if defined(ASMJIT_BUILD_X86)
#include "../x86/x86inst.h"
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_ARM)
#include "../arm/arminst.h"
#endif // ASMJIT_BUILD_ARM

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::CodeEmitter - Construction / Destruction]
// ============================================================================

CodeEmitter::CodeEmitter(uint32_t type) noexcept
  : _codeInfo(),
    _code(nullptr),
    _nextEmitter(nullptr),
    _type(static_cast<uint8_t>(type)),
    _destroyed(false),
    _finalized(false),
    _reserved(false),
    _lastError(kErrorNotInitialized),
    _privateData(0),
    _globalHints(0),
    _globalOptions(kOptionMaybeFailureCase),
    _options(0),
    _inlineComment(nullptr),
    _op4(),
    _op5(),
    _opExtra(),
    _none(),
    _nativeGpReg(),
    _nativeGpArray(nullptr) {}

CodeEmitter::~CodeEmitter() noexcept {
  if (_code) {
    _destroyed = true;
    _code->detach(this);
  }
}

// ============================================================================
// [asmjit::CodeEmitter - Events]
// ============================================================================

Error CodeEmitter::onAttach(CodeHolder* code) noexcept {
  _codeInfo = code->getCodeInfo();
  _lastError = kErrorOk;

  _globalHints = code->getGlobalHints();
  _globalOptions = code->getGlobalOptions();

  return kErrorOk;
}

Error CodeEmitter::onDetach(CodeHolder* code) noexcept {
  _codeInfo.reset();
  _finalized = false;
  _lastError = kErrorNotInitialized;

  _privateData = 0;
  _globalHints = 0;
  _globalOptions = kOptionMaybeFailureCase;

  _options = 0;
  _inlineComment = nullptr;
  _op4.reset();
  _op5.reset();
  _opExtra.reset();
  _nativeGpReg.reset();
  _nativeGpArray = nullptr;

  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeEmitter - Finalize]
// ============================================================================

Label CodeEmitter::getLabelByName(const char* name, size_t nameLength, uint32_t parentId) noexcept {
  return Label(_code ? _code->getLabelIdByName(name, nameLength, parentId) : static_cast<uint32_t>(0));
}

// ============================================================================
// [asmjit::CodeEmitter - Finalize]
// ============================================================================

Error CodeEmitter::finalize() {
  // Finalization does nothing by default, overridden by `CodeBuilder`.
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeEmitter - Error Handling]
// ============================================================================

Error CodeEmitter::setLastError(Error error, const char* message) {
  // This is fatal, CodeEmitter can't set error without being attached to `CodeHolder`.
  ASMJIT_ASSERT(_code != nullptr);

  // Special case used to reset the last error.
  if (error == kErrorOk) {
    _lastError = kErrorOk;
    _globalOptions &= ~kOptionMaybeFailureCase;
    return kErrorOk;
  }

  if (!message)
    message = DebugUtils::errorAsString(error);

  // Logging is skipped if the error is handled by `ErrorHandler`.
  ErrorHandler* handler = _code->_errorHandler;
  if (handler && handler->handleError(error, message, this))
    return error;

  // The handler->handleError() function may throw an exception or longjmp()
  // to terminate the execution of `setLastError()`. This is the reason why
  // we have delayed changing the `_error` member until now.
  _lastError = error;

  return error;
}

// ============================================================================
// [asmjit::CodeEmitter - Helpers]
// ============================================================================

bool CodeEmitter::isLabelValid(uint32_t id) const noexcept {
  size_t index = Operand::unpackId(id);
  return _code && index < _code->_labels.getLength();
}

Error CodeEmitter::commentf(const char* fmt, ...) {
  Error err = _lastError;
  if (err) return err;

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled) {
    va_list ap;
    va_start(ap, fmt);
    Error err = _code->_logger->logv(fmt, ap);
    va_end(ap);
  }
#else
  ASMJIT_UNUSED(fmt);
#endif

  return err;
}

Error CodeEmitter::commentv(const char* fmt, va_list ap) {
  Error err = _lastError;
  if (err) return err;

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled)
    err = _code->_logger->logv(fmt, ap);
#else
  ASMJIT_UNUSED(fmt);
  ASMJIT_UNUSED(ap);
#endif

  return err;
}

// ============================================================================
// [asmjit::CodeEmitter - Emit]
// ============================================================================

#define OP const Operand_&
#define NO _none

Error CodeEmitter::emit(uint32_t instId) { return _emit(instId, NO, NO, NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0) { return _emit(instId, o0, NO, NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1) { return _emit(instId, o0, o1, NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2) { return _emit(instId, o0, o1, o2, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3) { return _emit(instId, o0, o1, o2, o3); }

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4) {
  _op4 = o4;

  if (!o4.isNone()) _options |= kOptionOp4;
  return _emit(instId, o0, o1, o2, o3);
}

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, OP o5) {
  _op4 = o4;
  _op5 = o5;

  if (!o4.isNone()) _options |= kOptionOp4;
  if (!o5.isNone()) _options |= kOptionOp5;
  return _emit(instId, o0, o1, o2, o3);
}

Error CodeEmitter::emit(uint32_t instId, int o0) { return _emit(instId, Imm(o0), NO, NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, int o1) { return _emit(instId, o0, Imm(o1), NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, int o2) { return _emit(instId, o0, o1, Imm(o2), NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, int o3) { return _emit(instId, o0, o1, o2, Imm(o3)); }

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, int o4) {
  _options |= kOptionOp4;
  _op4 = Imm(o4);
  return _emit(instId, o0, o1, o2, o3);
}

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, int o5) {
  _op4 = o4;
  _op5 = Imm(o5);

  _options |= kOptionOp4 | kOptionOp5;
  return _emit(instId, o0, o1, o2, o3);
}

Error CodeEmitter::emit(uint32_t instId, int64_t o0) { return _emit(instId, Imm(o0), NO, NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, int64_t o1) { return _emit(instId, o0, Imm(o1), NO, NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, int64_t o2) { return _emit(instId, o0, o1, Imm(o2), NO); }
Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, int64_t o3) { return _emit(instId, o0, o1, o2, Imm(o3)); }

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, int64_t o4) {
  _options |= kOptionOp4;
  _op4 = Imm(o4);
  return _emit(instId, o0, o1, o2, o3);
}

Error CodeEmitter::emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, int64_t o5) {
  _op4 = o4;
  _op5 = Imm(o5);

  _options |= kOptionOp4 | kOptionOp5;
  return _emit(instId, o0, o1, o2, o3);
}

#undef NO
#undef OP

// ============================================================================
// [asmjit::CodeEmitter - Validation]
// ============================================================================

Error CodeEmitter::_validate(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) const noexcept {
#if !defined(ASMJIT_DISABLE_VALIDATION)
  Operand_ opArray[6];
  opArray[0].copyFrom(o0);
  opArray[1].copyFrom(o1);
  opArray[2].copyFrom(o2);
  opArray[3].copyFrom(o3);
  opArray[4].copyFrom(_op4);
  opArray[5].copyFrom(_op5);

  uint32_t archType = getArchType();
  uint32_t options = getGlobalOptions() | getOptions();

  if (!(options & CodeEmitter::kOptionOp4)) opArray[4].reset();
  if (!(options & CodeEmitter::kOptionOp5)) opArray[5].reset();

#if defined(ASMJIT_BUILD_X86)
  if (ArchInfo::isX86Family(archType))
    return X86Inst::validate(archType, instId, options, _opExtra, opArray, 6);
#endif

#if defined(ASMJIT_BUILD_ARM)
  if (ArchInfo::isArmFamily(archType))
    return ArmInst::validate(archType, instId, options, _opExtra, opArray, 6);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
#else
  return DebugUtils::errored(kErrorFeatureNotEnabled);
#endif // !ASMJIT_DISABLE_VALIDATION
}
} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"
