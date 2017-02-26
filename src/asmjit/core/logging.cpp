// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifndef ASMJIT_DISABLE_LOGGING

// [Dependencies]
#include "../core/codebuilder.h"
#include "../core/codecompiler.h"
#include "../core/codeholder.h"
#include "../core/codeemitter.h"
#include "../core/logging.h"
#include "../core/stringbuilder.h"
#include "../core/stringutils.h"
#include "../core/type.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86logging_p.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/armlogging_p.h"
#endif

ASMJIT_BEGIN_NAMESPACE

#if defined(ASMJIT_DISABLE_COMPILER)
class VirtReg;
#endif

// ============================================================================
// [asmjit::Logger - Construction / Destruction]
// ============================================================================

Logger::Logger() noexcept {
  _options = 0;
  std::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
}
Logger::~Logger() noexcept {}

// ============================================================================
// [asmjit::Logger - Logging]
// ============================================================================

Error Logger::logf(const char* fmt, ...) noexcept {
  Error err;
  std::va_list ap;

  va_start(ap, fmt);
  err = logv(fmt, ap);
  va_end(ap);

  return err;
}

Error Logger::logv(const char* fmt, std::va_list ap) noexcept {
  StringBuilderTmp<2048> sb;
  ASMJIT_PROPAGATE(sb.appendFormatVA(fmt, ap));
  return log(sb);
}

Error Logger::logBinary(const void* data, size_t size) noexcept {
  static const char prefix[] = "db ";

  StringBuilderTmp<256> sb;
  sb.appendString(prefix, ASMJIT_ARRAY_SIZE(prefix) - 1);

  size_t i = size;
  const uint8_t* s = static_cast<const uint8_t*>(data);

  while (i) {
    uint32_t n = uint32_t(std::min<size_t>(i, 16));
    sb.truncate(ASMJIT_ARRAY_SIZE(prefix) - 1);
    sb.appendHex(s, n);
    sb.appendChar('\n');
    ASMJIT_PROPAGATE(log(sb));
    s += n;
    i -= n;
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::Logger - Indentation]
// ============================================================================

void Logger::setIndentation(const char* indentation) noexcept {
  std::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
  if (!indentation)
    return;

  size_t length = StringUtils::strLen(indentation, ASMJIT_ARRAY_SIZE(_indentation) - 1);
  std::memcpy(_indentation, indentation, length);
}

// ============================================================================
// [asmjit::FileLogger - Construction / Destruction]
// ============================================================================

FileLogger::FileLogger(std::FILE* stream) noexcept
  : _stream(nullptr) { setStream(stream); }
FileLogger::~FileLogger() noexcept {}

// ============================================================================
// [asmjit::FileLogger - Logging]
// ============================================================================

Error FileLogger::_log(const char* buf, size_t len) noexcept {
  if (!_stream)
    return kErrorOk;

  if (len == Globals::kNullTerminated)
    len = strlen(buf);

  fwrite(buf, 1, len, _stream);
  return kErrorOk;
}

// ============================================================================
// [asmjit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger() noexcept {}
StringLogger::~StringLogger() noexcept {}

// ============================================================================
// [asmjit::StringLogger - Logging]
// ============================================================================

Error StringLogger::_log(const char* buf, size_t len) noexcept {
  return _stringBuilder.appendString(buf, len);
}

// ============================================================================
// [asmjit::Logging]
// ============================================================================

Error Logging::formatLabel(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t labelId) noexcept {

  ASMJIT_UNUSED(logOptions);

  const LabelEntry* le = emitter->getCode()->getLabelEntry(labelId);
  if (ASMJIT_UNLIKELY(!le))
    return sb.appendFormat("InvalidLabel[Id=%u]", labelId);

  if (le->hasName()) {
    if (le->hasParent()) {
      uint32_t parentId = le->getParentId();
      const LabelEntry* pe = emitter->getCode()->getLabelEntry(parentId);

      if (ASMJIT_UNLIKELY(!pe))
        ASMJIT_PROPAGATE(sb.appendFormat("InvalidLabel[Id=%u]", labelId));
      else if (ASMJIT_UNLIKELY(!pe->hasName()))
        ASMJIT_PROPAGATE(sb.appendFormat("L%u", Operand::unpackId(parentId)));
      else
        ASMJIT_PROPAGATE(sb.appendString(pe->getName()));

      ASMJIT_PROPAGATE(sb.appendChar('.'));
    }
    return sb.appendString(le->getName());
  }
  else {
    return sb.appendFormat("L%u", Operand::unpackId(labelId));
  }
}

Error Logging::formatRegister(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  uint32_t regType,
  uint32_t regId) noexcept {

  #ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archType))
    return X86Logging::formatRegister(sb, logOptions, emitter, archType, regType, regId);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archType))
    return ArmLogging::formatRegister(sb, logOptions, emitter, archType, regType, regId);
  #endif

  return kErrorInvalidArch;
}

Error Logging::formatOperand(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  const Operand_& op) noexcept {

  #ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archType))
    return X86Logging::formatOperand(sb, logOptions, emitter, archType, op);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archType))
    return ArmLogging::formatOperand(sb, logOptions, emitter, archType, op);
  #endif

  return kErrorInvalidArch;
}

Error Logging::formatInstruction(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  const Inst::Detail& detail, const Operand_* operands, uint32_t count) noexcept {

  #ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archType))
    return X86Logging::formatInstruction(sb, logOptions, emitter, archType, detail, operands, count);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archType))
    return ArmLogging::formatInstruction(sb, logOptions, emitter, archType, detail, operands, count);
  #endif

  return kErrorInvalidArch;
}

Error Logging::formatTypeId(StringBuilder& sb, uint32_t typeId) noexcept {
  if (typeId == Type::kIdVoid)
    return sb.appendString("void");

  if (!Type::isValid(typeId))
    return sb.appendString("unknown");

  const char* typeName = "unknown";
  uint32_t typeSize = Type::sizeOf(typeId);

  uint32_t baseId = Type::baseOf(typeId);
  switch (baseId) {
    case Type::kIdIntPtr : typeName = "intptr" ; break;
    case Type::kIdUIntPtr: typeName = "uintptr"; break;
    case Type::kIdI8     : typeName = "i8"     ; break;
    case Type::kIdU8     : typeName = "u8"     ; break;
    case Type::kIdI16    : typeName = "i16"    ; break;
    case Type::kIdU16    : typeName = "u16"    ; break;
    case Type::kIdI32    : typeName = "i32"    ; break;
    case Type::kIdU32    : typeName = "u32"    ; break;
    case Type::kIdI64    : typeName = "i64"    ; break;
    case Type::kIdU64    : typeName = "u64"    ; break;
    case Type::kIdF32    : typeName = "f32"    ; break;
    case Type::kIdF64    : typeName = "f64"    ; break;
    case Type::kIdF80    : typeName = "f80"    ; break;
    case Type::kIdMask8  : typeName = "mask8"  ; break;
    case Type::kIdMask16 : typeName = "mask16" ; break;
    case Type::kIdMask32 : typeName = "mask32" ; break;
    case Type::kIdMask64 : typeName = "mask64" ; break;
    case Type::kIdMmx32  : typeName = "mmx32"  ; break;
    case Type::kIdMmx64  : typeName = "mmx64"  ; break;
  }

  uint32_t baseSize = Type::sizeOf(baseId);
  if (typeSize > baseSize) {
    uint32_t count = typeSize / baseSize;
    return sb.appendFormat("%sx%u", typeName, unsigned(count));
  }
  else {
    return sb.appendString(typeName);
  }

}

#ifndef ASMJIT_DISABLE_BUILDER
static Error formatFuncValue(StringBuilder& sb, uint32_t logOptions, const CodeEmitter* emitter, FuncValue value) noexcept {
  uint32_t typeId = value.getTypeId();
  ASMJIT_PROPAGATE(Logging::formatTypeId(sb, typeId));

  if (value.isReg()) {
    ASMJIT_PROPAGATE(sb.appendChar('@'));
    ASMJIT_PROPAGATE(Logging::formatRegister(sb, logOptions, emitter, emitter->getArchType(), value.getRegType(), value.getRegId()));
  }

  if (value.isStack()) {
    ASMJIT_PROPAGATE(sb.appendFormat("@[%d]", int(value.getStackOffset())));
  }

  return kErrorOk;
}

static Error formatFuncRets(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  const FuncDetail& fd,
  VirtReg* const* vRegs) noexcept {

  if (!fd.hasRet())
    return sb.appendString("void");

  for (uint32_t i = 0; i < fd.getRetCount(); i++) {
    if (i) ASMJIT_PROPAGATE(sb.appendString(", "));
    ASMJIT_PROPAGATE(formatFuncValue(sb, logOptions, emitter, fd.getRet(i)));

    #ifndef ASMJIT_DISABLE_COMPILER
    if (vRegs) {
      static const char nullRet[] = "<none>";
      ASMJIT_PROPAGATE(sb.appendFormat(" %s", vRegs[i] ? vRegs[i]->getName() : nullRet));
    }
    #endif
  }

  return kErrorOk;
}

static Error formatFuncArgs(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  const FuncDetail& fd,
  VirtReg* const* vRegs) noexcept {

  uint32_t count = fd.getArgCount();
  if (!count)
    return sb.appendString("void");

  for (uint32_t i = 0; i < count; i++) {
    if (i) ASMJIT_PROPAGATE(sb.appendString(", "));
    ASMJIT_PROPAGATE(formatFuncValue(sb, logOptions, emitter, fd.getArg(i)));

    #ifndef ASMJIT_DISABLE_COMPILER
    if (vRegs) {
      static const char nullArg[] = "<none>";
      ASMJIT_PROPAGATE(sb.appendFormat(" %s", vRegs[i] ? vRegs[i]->getName() : nullArg));
    }
    #endif
  }

  return kErrorOk;
}

Error Logging::formatNode(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeBuilder* cb,
  const CBNode* node_) noexcept {

  if (node_->hasPosition() && (logOptions & Logger::kOptionNodePosition) != 0)
    ASMJIT_PROPAGATE(sb.appendFormat("<%05u> ", node_->getPosition()));

  switch (node_->getType()) {
    case CBNode::kNodeInst: {
      const CBInst* node = node_->as<CBInst>();
      ASMJIT_PROPAGATE(
        Logging::formatInstruction(sb, logOptions, cb,
          cb->getArchType(),
          node->getInstDetail(), node->getOpArray(), node->getOpCount()));
      break;
    }

    case CBNode::kNodeLabel: {
      const CBLabel* node = node_->as<CBLabel>();
      ASMJIT_PROPAGATE(sb.appendFormat("L%u:", Operand::unpackId(node->getId())));
      break;
    }

    case CBNode::kNodeData: {
      const CBData* node = node_->as<CBData>();
      ASMJIT_PROPAGATE(sb.appendFormat(".embed (%u bytes)", node->getSize()));
      break;
    }

    case CBNode::kNodeAlign: {
      const CBAlign* node = node_->as<CBAlign>();
      ASMJIT_PROPAGATE(
        sb.appendFormat(".align %u (%s)",
          node->getAlignment(),
          node->getMode() == kAlignCode ? "code" : "data"));
      break;
    }

    case CBNode::kNodeComment: {
      const CBComment* node = node_->as<CBComment>();
      ASMJIT_PROPAGATE(sb.appendFormat("; %s", node->getInlineComment()));
      break;
    }

    case CBNode::kNodeSentinel: {
      const CBSentinel* node = node_->as<CBSentinel>();
      const char* sentinelName = nullptr;

      switch (node->getSentinelType()) {
        case CBSentinel::kSentinelFuncEnd:
          sentinelName = "[FuncEnd]";
          break;

        default:
          sentinelName = "[Sentinel]";
          break;
      }

      ASMJIT_PROPAGATE(sb.appendString(sentinelName));
      break;
    }

    #ifndef ASMJIT_DISABLE_COMPILER
    case CBNode::kNodeFunc: {
      const CCFunc* node = node_->as<CCFunc>();

      ASMJIT_PROPAGATE(formatLabel(sb, logOptions, cb, node->getId()));
      ASMJIT_PROPAGATE(sb.appendString(": "));

      ASMJIT_PROPAGATE(formatFuncRets(sb, logOptions, cb, node->getDetail(), nullptr));
      ASMJIT_PROPAGATE(sb.appendString(" Func("));
      ASMJIT_PROPAGATE(formatFuncArgs(sb, logOptions, cb, node->getDetail(), node->getArgs()));
      ASMJIT_PROPAGATE(sb.appendString(")"));
      break;
    }

    case CBNode::kNodeFuncRet: {
      const CCFuncRet* node = node_->as<CCFuncRet>();
      ASMJIT_PROPAGATE(sb.appendString("[FuncRet]"));

      for (uint32_t i = 0; i < 2; i++) {
        const Operand_& op = node->_opArray[i];
        if (!op.isNone()) {
          ASMJIT_PROPAGATE(sb.appendString(i == 0 ? " " : ", "));
          ASMJIT_PROPAGATE(formatOperand(sb, logOptions, cb, cb->getArchType(), op));
        }
      }
      break;
    }

    case CBNode::kNodeFuncCall: {
      const CCFuncCall* node = node_->as<CCFuncCall>();
      ASMJIT_PROPAGATE(
        Logging::formatInstruction(sb, logOptions, cb,
          cb->getArchType(),
          node->getInstDetail(), node->getOpArray(), node->getOpCount()));
      break;
    }
    #endif

    default: {
      ASMJIT_PROPAGATE(sb.appendFormat("[User:%u]", node_->getType()));
      break;
    }
  }

  return kErrorOk;
}
#endif

Error Logging::formatLine(StringBuilder& sb, const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept {
  size_t currentLen = sb.getLength();
  size_t commentLen = comment ? StringUtils::strLen(comment, kMaxCommentLength) : 0;

  ASMJIT_ASSERT(binLen >= dispLen);
  const size_t kNoBinLen = std::numeric_limits<size_t>::max();

  if ((binLen != 0 && binLen != kNoBinLen) || commentLen) {
    size_t align = kMaxInstLength;
    char sep = ';';

    for (size_t i = (binLen == kNoBinLen); i < 2; i++) {
      size_t begin = sb.getLength();
      ASMJIT_PROPAGATE(sb.padEnd(align));

      if (sep) {
        ASMJIT_PROPAGATE(sb.appendChar(sep));
        ASMJIT_PROPAGATE(sb.appendChar(' '));
      }

      // Append binary data or comment.
      if (i == 0) {
        ASMJIT_PROPAGATE(sb.appendHex(binData, binLen - dispLen - imLen));
        ASMJIT_PROPAGATE(sb.appendChars('.', dispLen * 2));
        ASMJIT_PROPAGATE(sb.appendHex(binData + binLen - imLen, imLen));
        if (commentLen == 0) break;
      }
      else {
        ASMJIT_PROPAGATE(sb.appendString(comment, commentLen));
      }

      currentLen += sb.getLength() - begin;
      align += kMaxBinaryLength;
      sep = '|';
    }
  }

  return sb.appendChar('\n');
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif
