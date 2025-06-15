// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_LOGGING

#include "../core/formatter_p.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../arm/armformatter_p.h"
#include "../arm/a64operand.h"
#include "../arm/a64instapi_p.h"
#include "../arm/a64instdb_p.h"

#ifndef ASMJIT_NO_COMPILER
  #include "../core/compiler.h"
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

// arm::FormatterInternal - Format Feature
// =======================================

Error FormatterInternal::formatFeature(String& sb, uint32_t featureId) noexcept {
  // @EnumStringBegin{"enum": "CpuFeatures::ARM", "output": "sFeature", "strip": "k"}@
  static const char sFeatureString[] =
    "None\0"
    "ARMv6\0"
    "ARMv7\0"
    "ARMv8a\0"
    "THUMB\0"
    "THUMBv2\0"
    "ABLE\0"
    "ADERR\0"
    "AES\0"
    "AFP\0"
    "AIE\0"
    "AMU1\0"
    "AMU1_1\0"
    "ANERR\0"
    "ASIMD\0"
    "BF16\0"
    "BRBE\0"
    "BTI\0"
    "BWE\0"
    "CCIDX\0"
    "CHK\0"
    "CLRBHB\0"
    "CMOW\0"
    "CMPBR\0"
    "CONSTPACFIELD\0"
    "CPA\0"
    "CPA2\0"
    "CPUID\0"
    "CRC32\0"
    "CSSC\0"
    "CSV2\0"
    "CSV2_3\0"
    "CSV3\0"
    "D128\0"
    "DGH\0"
    "DIT\0"
    "DOTPROD\0"
    "DPB\0"
    "DPB2\0"
    "EBEP\0"
    "EBF16\0"
    "ECBHB\0"
    "ECV\0"
    "EDHSR\0"
    "EDSP\0"
    "F8E4M3\0"
    "F8E5M2\0"
    "F8F16MM\0"
    "F8F32MM\0"
    "FAMINMAX\0"
    "FCMA\0"
    "FGT\0"
    "FGT2\0"
    "FHM\0"
    "FLAGM\0"
    "FLAGM2\0"
    "FMAC\0"
    "FP\0"
    "FP16\0"
    "FP16CONV\0"
    "FP8\0"
    "FP8DOT2\0"
    "FP8DOT4\0"
    "FP8FMA\0"
    "FPMR\0"
    "FPRCVT\0"
    "FRINTTS\0"
    "GCS\0"
    "HACDBS\0"
    "HAFDBS\0"
    "HAFT\0"
    "HDBSS\0"
    "HBC\0"
    "HCX\0"
    "HPDS\0"
    "HPDS2\0"
    "I8MM\0"
    "IDIVA\0"
    "IDIVT\0"
    "ITE\0"
    "JSCVT\0"
    "LOR\0"
    "LRCPC\0"
    "LRCPC2\0"
    "LRCPC3\0"
    "LS64\0"
    "LS64_ACCDATA\0"
    "LS64_V\0"
    "LS64WB\0"
    "LSE\0"
    "LSE128\0"
    "LSE2\0"
    "LSFE\0"
    "LSUI\0"
    "LUT\0"
    "LVA\0"
    "LVA3\0"
    "MEC\0"
    "MOPS\0"
    "MPAM\0"
    "MTE\0"
    "MTE2\0"
    "MTE3\0"
    "MTE4\0"
    "MTE_ASYM_FAULT\0"
    "MTE_ASYNC\0"
    "MTE_CANONICAL_TAGS\0"
    "MTE_NO_ADDRESS_TAGS\0"
    "MTE_PERM_S1\0"
    "MTE_STORE_ONLY\0"
    "MTE_TAGGED_FAR\0"
    "MTPMU\0"
    "NMI\0"
    "NV\0"
    "NV2\0"
    "OCCMO\0"
    "PAN\0"
    "PAN2\0"
    "PAN3\0"
    "PAUTH\0"
    "PFAR\0"
    "PMU\0"
    "PMULL\0"
    "PRFMSLC\0"
    "RAS\0"
    "RAS1_1\0"
    "RAS2\0"
    "RASSA2\0"
    "RDM\0"
    "RME\0"
    "RNG\0"
    "RNG_TRAP\0"
    "RPRES\0"
    "RPRFM\0"
    "S1PIE\0"
    "S1POE\0"
    "S2PIE\0"
    "S2POE\0"
    "SB\0"
    "SCTLR2\0"
    "SEBEP\0"
    "SEL2\0"
    "SHA1\0"
    "SHA256\0"
    "SHA3\0"
    "SHA512\0"
    "SM3\0"
    "SM4\0"
    "SME\0"
    "SME2\0"
    "SME2_1\0"
    "SME2_2\0"
    "SME_AES\0"
    "SME_B16B16\0"
    "SME_B16F32\0"
    "SME_BI32I32\0"
    "SME_F16F16\0"
    "SME_F16F32\0"
    "SME_F32F32\0"
    "SME_F64F64\0"
    "SME_F8F16\0"
    "SME_F8F32\0"
    "SME_FA64\0"
    "SME_I16I32\0"
    "SME_I16I64\0"
    "SME_I8I32\0"
    "SME_LUTv2\0"
    "SME_MOP4\0"
    "SME_TMOP\0"
    "SPE\0"
    "SPE1_1\0"
    "SPE1_2\0"
    "SPE1_3\0"
    "SPE1_4\0"
    "SPE_ALTCLK\0"
    "SPE_CRR\0"
    "SPE_EFT\0"
    "SPE_FDS\0"
    "SPE_FPF\0"
    "SPE_SME\0"
    "SPECRES\0"
    "SPECRES2\0"
    "SPMU\0"
    "SSBS\0"
    "SSBS2\0"
    "SSVE_AES\0"
    "SSVE_BITPERM\0"
    "SSVE_FEXPA\0"
    "SSVE_FP8DOT2\0"
    "SSVE_FP8DOT4\0"
    "SSVE_FP8FMA\0"
    "SVE\0"
    "SVE2\0"
    "SVE2_1\0"
    "SVE2_2\0"
    "SVE_AES\0"
    "SVE_AES2\0"
    "SVE_B16B16\0"
    "SVE_BF16\0"
    "SVE_BFSCALE\0"
    "SVE_BITPERM\0"
    "SVE_EBF16\0"
    "SVE_ELTPERM\0"
    "SVE_F16MM\0"
    "SVE_F32MM\0"
    "SVE_F64MM\0"
    "SVE_I8MM\0"
    "SVE_PMULL128\0"
    "SVE_SHA3\0"
    "SVE_SM4\0"
    "SYSINSTR128\0"
    "SYSREG128\0"
    "THE\0"
    "TLBIOS\0"
    "TLBIRANGE\0"
    "TLBIW\0"
    "TME\0"
    "TRF\0"
    "UAO\0"
    "VFP_D32\0"
    "VHE\0"
    "VMID16\0"
    "WFXT\0"
    "XNX\0"
    "XS\0"
    "<Unknown>\0";

  static const uint16_t sFeatureIndex[] = {
    0, 5, 11, 17, 24, 30, 38, 43, 49, 53, 57, 61, 66, 73, 79, 85, 90, 95, 99,
    103, 109, 113, 120, 125, 131, 145, 149, 154, 160, 166, 171, 176, 183, 188,
    193, 197, 201, 209, 213, 218, 223, 229, 235, 239, 245, 250, 257, 264, 272,
    280, 289, 294, 298, 303, 307, 313, 320, 325, 328, 333, 342, 346, 354, 362,
    369, 374, 381, 389, 393, 400, 407, 412, 418, 422, 426, 431, 437, 442, 448,
    454, 458, 464, 468, 474, 481, 488, 493, 506, 513, 520, 524, 531, 536, 541,
    546, 550, 554, 559, 563, 568, 573, 577, 582, 587, 592, 607, 617, 636, 656,
    668, 683, 698, 704, 708, 711, 715, 721, 725, 730, 735, 741, 746, 750, 756,
    764, 768, 775, 780, 787, 791, 795, 799, 808, 814, 820, 826, 832, 838, 844,
    847, 854, 860, 865, 870, 877, 882, 889, 893, 897, 901, 906, 913, 920, 928,
    939, 950, 962, 973, 984, 995, 1006, 1016, 1026, 1035, 1046, 1057, 1067, 1077,
    1086, 1095, 1099, 1106, 1113, 1120, 1127, 1138, 1146, 1154, 1162, 1170, 1178,
    1186, 1195, 1200, 1205, 1211, 1220, 1233, 1244, 1257, 1270, 1282, 1286, 1291,
    1298, 1305, 1313, 1322, 1333, 1342, 1354, 1366, 1376, 1388, 1398, 1408, 1418,
    1427, 1440, 1449, 1457, 1469, 1479, 1483, 1490, 1500, 1506, 1510, 1514, 1518,
    1526, 1530, 1537, 1542, 1546, 1549
  };
  // @EnumStringEnd@

  return sb.append(sFeatureString + sFeatureIndex[Support::min<uint32_t>(featureId, uint32_t(CpuFeatures::ARM::kMaxValue) + 1)]);
}

// arm::FormatterInternal - Format Constants
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

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatShiftOp(String& sb, ShiftOp shiftOp) noexcept {
  const char* str = nullptr;
  switch (shiftOp) {
    case ShiftOp::kLSL: str = "lsl"; break;
    case ShiftOp::kLSR: str = "lsr"; break;
    case ShiftOp::kASR: str = "asr"; break;
    case ShiftOp::kROR: str = "ror"; break;
    case ShiftOp::kRRX: str = "rrx"; break;
    case ShiftOp::kMSL: str = "msl"; break;
    case ShiftOp::kUXTB: str = "uxtb"; break;
    case ShiftOp::kUXTH: str = "uxth"; break;
    case ShiftOp::kUXTW: str = "uxtw"; break;
    case ShiftOp::kUXTX: str = "uxtx"; break;
    case ShiftOp::kSXTB: str = "sxtb"; break;
    case ShiftOp::kSXTH: str = "sxth"; break;
    case ShiftOp::kSXTW: str = "sxtw"; break;
    case ShiftOp::kSXTX: str = "sxtx"; break;
    default: str = "<Unknown>"; break;
  }
  return sb.append(str);
}

// arm::FormatterInternal - Format Register
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

        ASMJIT_PROPAGATE(Formatter::formatVirtRegName(sb, vReg));
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
      case RegType::kVec8:
      case RegType::kVec16:
      case RegType::kVec32:
      case RegType::kVec64:
      case RegType::kVec128:
        letter = bhsdq[uint32_t(regType) - uint32_t(RegType::kVec8)];
        if (elementType) {
          letter = 'v';
        }
        break;

      case RegType::kGp32:
        if (Environment::is64Bit(arch)) {
          letter = 'w';

          if (rId == a64::Gp::kIdZr) {
            return sb.append("wzr", 3);
          }

          if (rId == a64::Gp::kIdSp) {
            return sb.append("wsp", 3);
          }
        }
        else {
          letter = 'r';
        }
        break;

      case RegType::kGp64:
        if (Environment::is64Bit(arch)) {
          if (rId == a64::Gp::kIdZr) {
            return sb.append("xzr", 3);
          }

          if (rId == a64::Gp::kIdSp) {
            return sb.append("sp", 2);
          }

          letter = 'x';
          break;
        }

        // X registers are undefined in 32-bit mode.
        [[fallthrough]];

      default:
        ASMJIT_PROPAGATE(sb.appendFormat("<Reg-%u>?%u", uint32_t(regType), rId));
        break;
    }

    if (letter)
      ASMJIT_PROPAGATE(sb.appendFormat("%c%u", letter, rId));
  }

  constexpr uint32_t kElementTypeCount = uint32_t(a64::VecElementType::kMaxValue) + 1;
  if (elementType) {
    elementType = Support::min(elementType, kElementTypeCount);

    FormatElementData elementData = formatElementDataTable[elementType];
    uint32_t elementCount = elementData.elementCount;

    if (regType == RegType::kVec64) {
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

    if (!first) {
      ASMJIT_PROPAGATE(sb.append(", "));
    }

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
    const Reg& reg = op.as<Reg>();

    uint32_t elementType = op._signature.getField<a64::Vec::kSignatureRegElementTypeMask>();
    uint32_t elementIndex = op.as<a64::Vec>().elementIndex();

    if (!op.as<a64::Vec>().hasElementIndex()) {
      elementIndex = 0xFFFFFFFFu;
    }

    return formatRegister(sb, flags, emitter, arch, reg.regType(), reg.id(), elementType, elementIndex);
  }

  if (op.isMem()) {
    const a64::Mem& m = op.as<a64::Mem>();
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
      // ARM really requires base.
      if (m.hasIndex() || m.hasOffset()) {
        ASMJIT_PROPAGATE(sb.append("<None>"));
      }
    }

    // The post index makes it look like there was another operand, but it's
    // still the part of AsmJit's `arm::Mem` operand so it's consistent with
    // other architectures.
    if (m.isPostIndex())
      ASMJIT_PROPAGATE(sb.append(']'));

    if (m.hasIndex()) {
      ASMJIT_PROPAGATE(sb.append(", "));
      ASMJIT_PROPAGATE(formatRegister(sb, flags, emitter, arch, m.indexType(), m.indexId()));
    }

    if (m.hasOffset()) {
      ASMJIT_PROPAGATE(sb.append(", "));

      int64_t off = int64_t(m.offset());
      uint32_t base = 10;

      if (Support::test(flags, FormatFlags::kHexOffsets) && uint64_t(off) > 9) {
        base = 16;
      }

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
      if (!m.isPreOrPost()) {
        ASMJIT_PROPAGATE(formatShiftOp(sb, m.shiftOp()));
      }
      ASMJIT_PROPAGATE(sb.appendFormat(" %u", m.shift()));
    }

    if (!m.isPostIndex()) {
      ASMJIT_PROPAGATE(sb.append(']'));
    }

    if (m.isPreIndex()) {
      ASMJIT_PROPAGATE(sb.append('!'));
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
    return formatRegisterList(sb, flags, emitter, arch, regList.regType(), regList.list());
  }

  return sb.append("<None>");
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
