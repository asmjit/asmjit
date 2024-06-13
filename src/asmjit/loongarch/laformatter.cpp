// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_LOGGING

#include "../core/misc_p.h"
#include "../core/support.h"
#include "../loongarch/laformatter_p.h"
#include "../loongarch/la64operand.h"
#include "../loongarch/la64instapi_p.h"
#include "../loongarch/la64instdb_p.h"

#ifndef ASMJIT_NO_COMPILER
  #include "../core/compiler.h"
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(la)

// la::FormatterInternal - Format Feature
// =======================================

Error FormatterInternal::formatFeature(String& sb, uint32_t featureId) noexcept {
  // @EnumStringBegin{"enum": "CpuFeatures::LOONGARCH", "output": "sFeature", "strip": "k"}@
  static const char sFeatureString[] =
    "None\0"
    "LOONGARCH64\0"
    "LOONGARCH32\0"
    "LSX\0"
    "LASX\0"
    "<Unknown>\0";

  static const uint16_t sFeatureIndex[] = {
    0, 5, 17, 29, 33
  };
  // @EnumStringEnd@

  return sb.append(sFeatureString + sFeatureIndex[Support::min<uint32_t>(featureId, uint32_t(CpuFeatures::LOONGARCH::kMaxValue) + 1)]);
}

// la::FormatterInternal - Format Constants
// =========================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatCondCode(String& sb, CondCode cc) noexcept {
  static const char condCodeData[] =
    "al\0" "na\0"
    "eq\0" "ne\0"
    "hs\0" "lo\0" "mi\0" "pl\0" "vs\0" "vc\0"
    "hi\0" "ls\0" "ge\0" "lt\0" "gt\0" "le\0"
    "<Unknown>";
  return sb.append(condCodeData + Support::min<uint32_t>(uint32_t(cc), 16u) * 3);
}

//FIXME
ASMJIT_FAVOR_SIZE Error FormatterInternal::formatShiftOp(String& sb, ShiftOp shiftOp) noexcept {
  const char* str = nullptr;
  switch (shiftOp) {
    case ShiftOp::kSLL: str = "lsl"; break;
    case ShiftOp::kSRL: str = "lsr"; break;
    case ShiftOp::kSRA: str = "asr"; break;
    case ShiftOp::kRORT: str = "ror"; break;
    default: str = "<Unknown>"; break;
  }
  return sb.append(str);
}

// la::FormatterInternal - Format Register
// ========================================

struct FormatElementData {
  char letter;
  uint8_t elementCount;
  uint8_t onlyIndex;
  uint8_t reserved;
};

static constexpr FormatElementData formatElementDataTable[9] = {
  { '?' , 0 , 0, 0 }, // None
  { 'b' , 16, 0, 0 }, // bX or b[index]
  { 'h' , 8 , 0, 0 }, // hX or h[index]
  { 's' , 4 , 0, 0 }, // sX or s[index]
  { 'd' , 2 , 0, 0 }, // dX or d[index]
  { 'b' , 4 , 1, 0 }, // ?? or b4[index]
  { 'h' , 2 , 1, 0 }, // ?? or h2[index]
  { '?' , 0 , 0, 0 }, // invalid (possibly stored in Operand)
  { '?' , 0 , 0, 0 }  // invalid (never stored in Operand, bug...)
};

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatRegister(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType regType,
  uint32_t rId,
  uint32_t elementType,
  uint32_t elementIndex) noexcept {

  DebugUtils::unused(flags);
  DebugUtils::unused(arch);

  static const char bhsdq[] = "bhsdq";

  bool virtRegFormatted = false;

#ifndef ASMJIT_NO_COMPILER
  if (Operand::isVirtId(rId)) {
    if (emitter && emitter->isCompiler()) {
      const BaseCompiler* cc = static_cast<const BaseCompiler*>(emitter);
      if (cc->isVirtIdValid(rId)) {
        VirtReg* vReg = cc->virtRegById(rId);
        ASMJIT_ASSERT(vReg != nullptr);

        const char* name = vReg->name();
        if (name && name[0] != '\0')
          ASMJIT_PROPAGATE(sb.append(name));
        else
          ASMJIT_PROPAGATE(sb.appendFormat("%%%u", unsigned(Operand::virtIdToIndex(rId))));

        virtRegFormatted = true;
      }
    }
  }
#else
  DebugUtils::unused(emitter, flags);
#endif

  if (!virtRegFormatted) {
    char letter = '\0';
    switch (regType) {
      case RegType::kLA_VecB:
      case RegType::kLA_VecH:
      case RegType::kLA_VecS:
      case RegType::kLA_VecD:
      case RegType::kLA_VecV:
        letter = bhsdq[uint32_t(regType) - uint32_t(RegType::kLA_VecB)];
        if (elementType)
          letter = 'v';
        break;

      case RegType::kLA_GpW:
        if (Environment::is64Bit(arch)) {
          letter = 'w';

          if (rId == la64::Gp::kIdZr)
            return sb.append("wzr", 3);

          if (rId == la64::Gp::kIdSp)
            return sb.append("wsp", 3);
        }
        else {
          letter = 'r';
        }
        break;

      case RegType::kLA_GpX:
        if (Environment::is64Bit(arch)) {
          if (rId == la64::Gp::kIdZr)
            return sb.append("xzr", 3);
          if (rId == la64::Gp::kIdSp)
            return sb.append("sp", 2);

          letter = 'x';
          break;
        }

        // X registers are undefined in 32-bit mode.
        ASMJIT_FALLTHROUGH;

      default:
        ASMJIT_PROPAGATE(sb.appendFormat("<Reg-%u>?%u", uint32_t(regType), rId));
        break;
    }

    if (letter)
      ASMJIT_PROPAGATE(sb.appendFormat("%c%u", letter, rId));
  }

  constexpr uint32_t kElementTypeCount = uint32_t(la64::VecElementType::kMaxValue) + 1;
  if (elementType) {
    elementType = Support::min(elementType, kElementTypeCount);

    FormatElementData elementData = formatElementDataTable[elementType];
    uint32_t elementCount = elementData.elementCount;

    if (regType == RegType::kLA_VecD) {
      elementCount /= 2u;
    }

    ASMJIT_PROPAGATE(sb.append('.'));
    if (elementCount) {
      ASMJIT_PROPAGATE(sb.appendUInt(elementCount));
    }
    ASMJIT_PROPAGATE(sb.append(elementData.letter));
  }

  if (elementIndex != 0xFFFFFFFFu) {
    ASMJIT_PROPAGATE(sb.appendFormat("[%u]", elementIndex));
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatRegisterList(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType regType,
  uint32_t rMask) noexcept {

  bool first = true;

  ASMJIT_PROPAGATE(sb.append('{'));
  while (rMask != 0u) {
    uint32_t start = Support::ctz(rMask);
    uint32_t count = 0u;

    uint32_t mask = 1u << start;
    do {
      rMask &= ~mask;
      mask <<= 1u;
      count++;
    } while (rMask & mask);

    if (!first)
      ASMJIT_PROPAGATE(sb.append(", "));

    ASMJIT_PROPAGATE(formatRegister(sb, flags, emitter, arch, regType, start, 0, 0xFFFFFFFFu));
    if (count >= 2u) {
      ASMJIT_PROPAGATE(sb.append('-'));
      ASMJIT_PROPAGATE(formatRegister(sb, flags, emitter, arch, regType, start + count - 1, 0, 0xFFFFFFFFu));
    }

    first = false;
  }
  ASMJIT_PROPAGATE(sb.append('}'));

  return kErrorOk;
}

// a64::FormatterInternal - Format Operand
// =======================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatOperand(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept {

  if (op.isReg()) {
    const BaseReg& reg = op.as<BaseReg>();

    uint32_t elementType = op._signature.getField<BaseVec::kSignatureRegElementTypeMask>();
    uint32_t elementIndex = op.as<BaseVec>().elementIndex();

    if (!op.as<BaseVec>().hasElementIndex())
      elementIndex = 0xFFFFFFFFu;

    return formatRegister(sb, flags, emitter, arch, reg.type(), reg.id(), elementType, elementIndex);
  }

  if (op.isMem()) {
    const Mem& m = op.as<Mem>();
    ASMJIT_PROPAGATE(sb.append('['));

    if (m.hasBase()) {
      if (m.hasBaseLabel()) {
        ASMJIT_PROPAGATE(Formatter::formatLabel(sb, flags, emitter, m.baseId()));
      }
      else {
        FormatFlags modifiedFlags = flags;
        if (m.isRegHome()) {
          ASMJIT_PROPAGATE(sb.append('&'));
          modifiedFlags &= ~FormatFlags::kRegCasts;
        }
        ASMJIT_PROPAGATE(formatRegister(sb, modifiedFlags, emitter, arch, m.baseType(), m.baseId()));
      }
    }
    else {
      if (m.hasIndex() || m.hasOffset()) {
        ASMJIT_PROPAGATE(sb.append("<None>"));
      }
    }

    // The post index makes it look like there was another operand, but it's
    // still the part of AsmJit's `la::Mem` operand so it's consistent with
    // other architectures.

    if (m.hasIndex()) {
      ASMJIT_PROPAGATE(sb.append(", "));
      ASMJIT_PROPAGATE(formatRegister(sb, flags, emitter, arch, m.indexType(), m.indexId()));
    }

    if (m.hasOffset()) {
      ASMJIT_PROPAGATE(sb.append(", "));

      int64_t off = int64_t(m.offset());
      uint32_t base = 10;

      if (Support::test(flags, FormatFlags::kHexOffsets) && uint64_t(off) > 9)
        base = 16;

      if (base == 10) {
        ASMJIT_PROPAGATE(sb.appendInt(off, base));
      }
      else {
        ASMJIT_PROPAGATE(sb.append("0x"));
        ASMJIT_PROPAGATE(sb.appendUInt(uint64_t(off), base));
      }
    }

    if (m.hasShift()) {
      ASMJIT_PROPAGATE(sb.append(' '));
      if (!m.isPreOrPost())
        ASMJIT_PROPAGATE(formatShiftOp(sb, m.shiftOp()));
      ASMJIT_PROPAGATE(sb.appendFormat(" %u", m.shift()));
    }

    return kErrorOk;
  }

  if (op.isImm()) {
    const Imm& i = op.as<Imm>();
    int64_t val = i.value();
    uint32_t predicate = i.predicate();

    if (predicate) {
      ASMJIT_PROPAGATE(formatShiftOp(sb, ShiftOp(predicate)));
      ASMJIT_PROPAGATE(sb.append(' '));
    }

    if (Support::test(flags, FormatFlags::kHexImms) && uint64_t(val) > 9) {
      ASMJIT_PROPAGATE(sb.append("0x"));
      return sb.appendUInt(uint64_t(val), 16);
    }
    else {
      return sb.appendInt(val, 10);
    }
  }

  if (op.isLabel()) {
    return Formatter::formatLabel(sb, flags, emitter, op.id());
  }

  if (op.isRegList()) {
    const BaseRegList& regList = op.as<BaseRegList>();
    return formatRegisterList(sb, flags, emitter, arch, regList.type(), regList.list());
  }

  return sb.append("<None>");
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
