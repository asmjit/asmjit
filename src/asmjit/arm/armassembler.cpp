// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

#include "../core/cpuinfo.h"
#include "../core/logging.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../arm/armassembler.h"
#include "../arm/armlogging_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

// ============================================================================
// [asmjit::Assembler - Construction / Destruction]
// ============================================================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  if (code)
    code->attach(this);
}
Assembler::~Assembler() noexcept {}

// ============================================================================
// [asmjit::Assembler - Helpers]
// ============================================================================

#define ENC_OPS1(OP0)                     ((Operand::kOp##OP0))
#define ENC_OPS2(OP0, OP1)                ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3))
#define ENC_OPS3(OP0, OP1, OP2)           ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6))
#define ENC_OPS4(OP0, OP1, OP2, OP3)      ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6) + ((Operand::kOp##OP3) << 9))
#define ENC_OPS5(OP0, OP1, OP2, OP3, OP4) ((Operand::kOp##OP0) + ((Operand::kOp##OP1) << 3) + ((Operand::kOp##OP2) << 6) + ((Operand::kOp##OP3) << 9) + ((Operand::kOp##OP4) << 12))

// ============================================================================
// [asmjit::Assembler - Emit]
// ============================================================================

Error Assembler::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
  uint8_t* cursor = _bufferPtr;
  uint32_t options = uint32_t(instId >= ArmInst::_kIdCount) | instOptions() | globalInstOptions();

  const InstDB::InstInfo* instInfo = ArmInstDB::instData + instId;
  const InstDB::CommonInfo* commonInfo;

  // Signature of the first 3 operands.
  uint32_t isign3 = o0.opType() + (o1.opType() << 3) + (o2.opType() << 6);

  if (ASMJIT_UNLIKELY(options & BaseInst::kOptionReserved)) {
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

  commonInfo = &instData->commonInfo();

  switch (instData->encodingType()) {
  }

  // --------------------------------------------------------------------------
  // [Done]
  // --------------------------------------------------------------------------

EmitDone:
  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _emitLog(instId, options, o0, o1, o2, o3, relSize, immSize, cursor);
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
// [asmjit::Assembler - Align]
// ============================================================================

Error Assembler::align(uint32_t alignMode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled))) {
    _code->_logger->logf("%s.align %u\n", _code->_logger->getIndentation(), alignment);
  }
  #endif

  if (ASMJIT_UNLIKELY(alignMode >= kAlignCount))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment <= 1)
    return kErrorOk;

  if (ASMJIT_UNLIKELY(alignment > Globals::kMaxAlignment || !Support::isPowerOf2(alignment)))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  size_t offset = offset();
  uint32_t i = uint32_t(Support::alignUpDiff<size_t>(offset, alignment));

  if (i == 0)
    return kErrorOk;

  CodeBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, i));

  constexpr uint32_t kNopT16 = 0x0000BF00u; // [10111111|00000000].
  constexpr uint32_t kNopT32 = 0xF3AF8000u; // [11110011|10101111|10000000|00000000].
  constexpr uint32_t kNopA32 = 0xE3AF8000u; // [Cond0011|00100000|11110000|00000000].

  switch (alignMode) {
    case kAlignCode: {
      if (isInThumbMode()) {
        uint32_t pattern = 0;
        if (ASMJIT_UNLIKELY(offset & 0x1u))
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
        if (ASMJIT_UNLIKELY(offset & 0x3u))
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
// [asmjit::Assembler - Events]
// ============================================================================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  uint32_t archId = code->archId();
  if (archId != ArchInfo::kIdA32)
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));
  _gpRegInfo.setSignature(ArmGpw::kSignature);
  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_BUILD_ARM
