// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

// [Dependencies]
#include "../core/cpuinfo.h"
#include "../core/intutils.h"
#include "../core/logging.h"
#include "../core/memutils.h"
#include "../core/misc_p.h"
#include "../arm/armassembler.h"
#include "../arm/armlogging_p.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::A32Assembler - Construction / Destruction]
// ============================================================================

A32Assembler::A32Assembler(CodeHolder* code) noexcept : Assembler() {
  if (code)
    code->attach(this);
}
A32Assembler::~A32Assembler() noexcept {}

// ============================================================================
// [asmjit::A32Assembler - Helpers]
// ============================================================================

#define ENC_OPS1(OP0)                     ((Operand::kOp##OP0))
#define ENC_OPS2(OP0, OP1)                ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3))
#define ENC_OPS3(OP0, OP1, OP2)           ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6))
#define ENC_OPS4(OP0, OP1, OP2, OP3)      ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6) + ((Operand::kOp##OP3) << 9))
#define ENC_OPS5(OP0, OP1, OP2, OP3, OP4) ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6) + ((Operand::kOp##OP3) << 9) + ((Operand::kOp##OP4) << 12))

// ============================================================================
// [asmjit::A32Assembler - Emit]
// ============================================================================

Error A32Assembler::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
  uint8_t* cursor = _bufferPtr;
  uint32_t options = uint32_t(instId >= ArmInst::_kIdCount) | getInstOptions() | getGlobalInstOptions();

  const ArmInst* instData = ArmInstDB::instData + instId;
  const ArmInst::CommonData* commonData;

  // Signature of the first 3 operands.
  uint32_t isign3 = o0.getOp() + (o1.getOp() << 3) + (o2.getOp() << 6);

  if (ASMJIT_UNLIKELY(options & Inst::kOptionReserved)) {
    if (ASMJIT_UNLIKELY(!_code))
      return DebugUtils::errored(kErrorNotInitialized);

    // Unknown instruction.
    if (ASMJIT_UNLIKELY(instId >= ArmInst::_kIdCount))
      goto InvalidInstruction;

    // Strict validation.
    #ifndef ASMJIT_DISABLE_INST_API
    if (hasEmitterOption(kOptionStrictValidation))
      ASMJIT_PROPAGATE(_validate(instId, o0, o1, o2, o3));
    #endif
  }

  // --------------------------------------------------------------------------
  // [Encoding Scope]
  // --------------------------------------------------------------------------

  commonData = &instData->getCommonData();

  switch (instData->getEncodingType()) {
  }

  // --------------------------------------------------------------------------
  // [Done]
  // --------------------------------------------------------------------------

EmitDone:
  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _emitLog(instId, options, o0, o1, o2, o3, relSize, imLen, cursor);
  #endif

  resetInstOptions();
  resetInlineComment();

  _bufferPtr = cursor;
  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Error Cases]
  // --------------------------------------------------------------------------

#define ERROR_HANDLER(ERROR)                \
ERROR:                                      \
  err = DebugUtils::errored(kError##ERROR); \
  goto Failed;

ERROR_HANDLER(NoHeapMemory)

#undef ERROR_HANDLER

Failed:
  return _failedInstruction(err, instId, options, o0, o1, o2, o3);
}

// ============================================================================
// [asmjit::A32Assembler - Align]
// ============================================================================

Error A32Assembler::align(uint32_t mode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logf("%s.align %u\n", _code->_logger->getIndentation(), alignment);
  #endif

  if (ASMJIT_UNLIKELY(mode >= kAlignCount))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment <= 1)
    return kErrorOk;

  if (ASMJIT_UNLIKELY(alignment > Globals::kMaxAlignment || !IntUtils::isPowerOf2(alignment)))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  size_t offset = getOffset();
  uint32_t i = uint32_t(IntUtils::alignUpDiff<size_t>(offset, alignment));

  if (i == 0)
    return kErrorOk;

  AsmBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, i));

  constexpr uint32_t kNopT16 = 0x0000BF00U; // [10111111|00000000].
  constexpr uint32_t kNopT32 = 0xF3AF8000U; // [11110011|10101111|10000000|00000000].
  constexpr uint32_t kNopA32 = 0xE3AF8000U; // [Cond0011|00100000|11110000|00000000].

  switch (mode) {
    case kAlignCode: {
      if (isInThumbMode()) {
        uint32_t pattern = 0;
        if (ASMJIT_UNLIKELY(offset & 0x1U))
          return DebugUtils::errored(kErrorInvalidState);

        while (i >= 4) {
          writer.emit32uLE(kNopT32);
          i -= 4;
        }

        if (i >= 2) {
          writer.emit16uLE(kNopT16);
          i -= 2;
        }
      }
      else {
        uint32_t pattern = 0;
        if (ASMJIT_UNLIKELY(offset & 0x3U))
          return DebugUtils::errored(kErrorInvalidState);

        while (i >= 4) {
          writer.emit32uLE(kNopA32);
          i -= 4;
        }
      }

      ASMJIT_ASSERT(i == 0);
      break;
    }

    case kAlignData:
    case kAlignZero:
      writer.emitZeros(i);
      break;
  }

  writer.done(this);
  return kErrorOk;
}

// ============================================================================
// [asmjit::A32Assembler - Events]
// ============================================================================

Error A32Assembler::onAttach(CodeHolder* code) noexcept {
  uint32_t archType = code->getArchType();
  if (archType != ArchInfo::kTypeA32)
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));
  _gpRegInfo.setSignature(ArmGpw::kSignature);
  return kErrorOk;
}

Error A32Assembler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_ARM
