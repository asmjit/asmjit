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
#include "../core/misc_p.h"
#include "../core/stringutils.h"

#include "../x86/x86inst.h"
#include "../x86/x86logging_p.h"
#include "../x86/x86operand.h"

#ifndef ASMJIT_DISABLE_COMPILER
  #include "../core/codecompiler.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::X86Logging - Constants]
// ============================================================================

struct RegFormatInfo {
  struct TypeEntry {
    uint8_t index;
  };

  struct NameEntry {
    uint8_t count;
    uint8_t formatIndex;
    uint8_t specialIndex;
    uint8_t specialCount;
  };

  TypeEntry typeEntries[Reg::kRegMax + 1];
  char typeStrings[128 - 32];

  NameEntry nameEntries[Reg::kRegMax + 1];
  char nameStrings[280];
};

template<uint32_t X>
struct X86RegFormatInfo_T {
  enum {
    kTypeIndex    = X == X86Reg::kRegGpbLo ? 1   :
                    X == X86Reg::kRegGpbHi ? 8   :
                    X == X86Reg::kRegGpw   ? 15  :
                    X == X86Reg::kRegGpd   ? 19  :
                    X == X86Reg::kRegGpq   ? 23  :
                    X == X86Reg::kRegXmm   ? 27  :
                    X == X86Reg::kRegYmm   ? 31  :
                    X == X86Reg::kRegZmm   ? 35  :
                    X == X86Reg::kRegRip   ? 39  :
                    X == X86Reg::kRegSeg   ? 43  :
                    X == X86Reg::kRegFp    ? 47  :
                    X == X86Reg::kRegMm    ? 50  :
                    X == X86Reg::kRegK     ? 53  :
                    X == X86Reg::kRegBnd   ? 55  :
                    X == X86Reg::kRegCr    ? 59  :
                    X == X86Reg::kRegDr    ? 62  : 0,

    kFormatIndex  = X == X86Reg::kRegGpbLo ? 1   :
                    X == X86Reg::kRegGpbHi ? 6   :
                    X == X86Reg::kRegGpw   ? 11  :
                    X == X86Reg::kRegGpd   ? 16  :
                    X == X86Reg::kRegGpq   ? 21  :
                    X == X86Reg::kRegXmm   ? 25  :
                    X == X86Reg::kRegYmm   ? 31  :
                    X == X86Reg::kRegZmm   ? 37  :
                    X == X86Reg::kRegRip   ? 43  :
                    X == X86Reg::kRegSeg   ? 49  :
                    X == X86Reg::kRegFp    ? 55  :
                    X == X86Reg::kRegMm    ? 60  :
                    X == X86Reg::kRegK     ? 65  :
                    X == X86Reg::kRegBnd   ? 69  :
                    X == X86Reg::kRegCr    ? 75  :
                    X == X86Reg::kRegDr    ? 80  : 0,

    kSpecialIndex = X == X86Reg::kRegGpbLo ? 96  :
                    X == X86Reg::kRegGpbHi ? 128 :
                    X == X86Reg::kRegGpw   ? 161 :
                    X == X86Reg::kRegGpd   ? 160 :
                    X == X86Reg::kRegGpq   ? 192 :
                    X == X86Reg::kRegRip   ? 85  :
                    X == X86Reg::kRegSeg   ? 224 : 0,

    kSpecialCount = X == X86Reg::kRegGpbLo ? 8   :
                    X == X86Reg::kRegGpbHi ? 4   :
                    X == X86Reg::kRegGpw   ? 8   :
                    X == X86Reg::kRegGpd   ? 8   :
                    X == X86Reg::kRegGpq   ? 8   :
                    X == X86Reg::kRegRip   ? 1   :
                    X == X86Reg::kRegSeg   ? 7   : 0
  };
};

#define ASMJIT_REG_TYPE_ENTRY(TYPE) {      \
  X86RegFormatInfo_T<TYPE>::kTypeIndex     \
}

#define ASMJIT_REG_NAME_ENTRY(TYPE) {      \
  X86RegTraits<TYPE>::kCount,              \
  X86RegFormatInfo_T<TYPE>::kFormatIndex,  \
  X86RegFormatInfo_T<TYPE>::kSpecialIndex, \
  X86RegFormatInfo_T<TYPE>::kSpecialCount  \
}

static const RegFormatInfo x86RegFormatInfo = {
  // Register type entries and strings.
  { ASMJIT_TABLE_32(ASMJIT_REG_TYPE_ENTRY, 0) },

  "\0"             // #0
  "gpb.lo\0"       // #1
  "gpb.hi\0"       // #8
  "gpw\0"          // #15
  "gpd\0"          // #19
  "gpq\0"          // #23
  "xmm\0"          // #27
  "ymm\0"          // #31
  "zmm\0"          // #35
  "rip\0"          // #39
  "seg\0"          // #43
  "fp\0"           // #47
  "mm\0"           // #50
  "k\0"            // #53
  "bnd\0"          // #55
  "cr\0"           // #59
  "dr\0",          // #62

  // Register name entries and strings.
  { ASMJIT_TABLE_32(ASMJIT_REG_NAME_ENTRY, 0) },

  "\0"
  "r%ub\0"         // #1
  "r%uh\0"         // #6
  "r%uw\0"         // #11
  "r%ud\0"         // #16
  "r%u\0"          // #21
  "xmm%u\0"        // #25
  "ymm%u\0"        // #31
  "zmm%u\0"        // #37
  "rip%u\0"        // #43
  "seg%u\0"        // #49
  "fp%u\0"         // #55
  "mm%u\0"         // #60
  "k%u\0"          // #65
  "bnd%u\0"        // #69
  "cr%u\0"         // #75
  "dr%u\0"         // #80

  "rip\0"          // #85
  "\0\0\0\0\0\0\0" // #89

  "al\0\0" "cl\0\0" "dl\0\0" "bl\0\0" "spl\0"  "bpl\0"  "sil\0"  "dil\0" // #96
  "ah\0\0" "ch\0\0" "dh\0\0" "bh\0\0" "n/a\0"  "n/a\0"  "n/a\0"  "n/a\0" // #128
  "eax\0"  "ecx\0"  "edx\0"  "ebx\0"  "esp\0"  "ebp\0"  "esi\0"  "edi\0" // #160
  "rax\0"  "rcx\0"  "rdx\0"  "rbx\0"  "rsp\0"  "rbp\0"  "rsi\0"  "rdi\0" // #192
  "n/a\0"  "es\0\0" "cs\0\0" "ss\0\0" "ds\0\0" "fs\0\0" "gs\0\0" "n/a\0" // #224
};
#undef ASMJIT_REG_NAME_ENTRY
#undef ASMJIT_REG_TYPE_ENTRY

static const char* x86GetAddressSizeString(uint32_t size) noexcept {
  switch (size) {
    case 1 : return "byte ";
    case 2 : return "word ";
    case 4 : return "dword ";
    case 6 : return "fword ";
    case 8 : return "qword ";
    case 10: return "tword ";
    case 16: return "oword ";
    case 32: return "yword ";
    case 64: return "zword ";
    default: return "";
  }
}

// ============================================================================
// [asmjit::X86Logging - Format Operand]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Logging::formatOperand(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  const Operand_& op) noexcept {

  if (op.isReg())
    return formatRegister(sb, logOptions, emitter, archType, op.as<Reg>().getType(), op.as<Reg>().getId());

  if (op.isMem()) {
    const X86Mem& m = op.as<X86Mem>();
    ASMJIT_PROPAGATE(sb.appendString(x86GetAddressSizeString(m.getSize())));

    // Segment override prefix.
    uint32_t seg = m.getSegmentId();
    if (seg != X86Seg::kIdNone && seg < X86Seg::kIdCount)
      ASMJIT_PROPAGATE(sb.appendFormat("%s:", x86RegFormatInfo.nameStrings + 224 + seg * 4));

    ASMJIT_PROPAGATE(sb.appendChar('['));
    if (m.isAbs())
      ASMJIT_PROPAGATE(sb.appendString("abs "));

    char opSign = '\0';
    if (m.hasBase()) {
      opSign = '+';
      if (m.hasBaseLabel()) {
        ASMJIT_PROPAGATE(Logging::formatLabel(sb, logOptions, emitter, m.getBaseId()));
      }
      else {
        uint32_t opt = logOptions;
        if (m.isRegHome()) {
          ASMJIT_PROPAGATE(sb.appendString("&"));
          opt &= ~Logger::kOptionRegCasts;
        }
        ASMJIT_PROPAGATE(formatRegister(sb, opt, emitter, archType, m.getBaseType(), m.getBaseId()));
      }
    }

    if (m.hasIndex()) {
      if (opSign)
        ASMJIT_PROPAGATE(sb.appendChar(opSign));

      opSign = '+';
      ASMJIT_PROPAGATE(formatRegister(sb, logOptions, emitter, archType, m.getIndexType(), m.getIndexId()));
      if (m.hasShift())
        ASMJIT_PROPAGATE(sb.appendFormat("*%u", 1 << m.getShift()));
    }

    uint64_t off = uint64_t(m.getOffset());
    if (off) {
      if (int64_t(off) < 0) {
        opSign = '-';
        off = ~off + 1;
      }

      if (opSign)
        ASMJIT_PROPAGATE(sb.appendChar(opSign));

      uint32_t base = 10;
      if ((logOptions & Logger::kOptionHexOffsets) != 0 && off > 9) {
        ASMJIT_PROPAGATE(sb.appendString("0x", 2));
        base = 16;
      }

      ASMJIT_PROPAGATE(sb.appendUInt(off, base));
    }

    return sb.appendChar(']');
  }

  if (op.isImm()) {
    const Imm& i = op.as<Imm>();
    int64_t val = i.getInt64();

    if ((logOptions & Logger::kOptionHexConsts) != 0 && uint64_t(val) > 9)
      return sb.appendUInt(uint64_t(val), 16);
    else
      return sb.appendInt(val, 10);
  }

  if (op.isLabel()) {
    return Logging::formatLabel(sb, logOptions, emitter, op.getId());
  }

  return sb.appendString("<None>");
}

// ============================================================================
// [asmjit::X86Logging - Format Immediate (Extension)]
// ============================================================================

static constexpr char kImmCharStart = '{';
static constexpr char kImmCharEnd   = '}';
static constexpr char kImmCharOr    = '|';

struct ImmBits {
  enum Mode : uint32_t {
    kModeLookup = 0,
    kModeFormat = 1
  };

  uint8_t mask;
  uint8_t shift;
  uint8_t mode;
  char text[48 - 3];
};

ASMJIT_FAVOR_SIZE static Error X86Logging_formatImmShuf(StringBuilder& sb, uint32_t u8, uint32_t bits, uint32_t count) noexcept {
  uint32_t mask = (1 << bits) - 1;

  for (uint32_t i = 0; i < count; i++, u8 >>= bits) {
    uint32_t value = u8 & mask;
    ASMJIT_PROPAGATE(sb.appendChar(i == 0 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.appendUInt(value));
  }

  if (kImmCharEnd)
    ASMJIT_PROPAGATE(sb.appendChar(kImmCharEnd));

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE static Error X86Logging_formatImmBits(StringBuilder& sb, uint32_t u8, const ImmBits* bits, uint32_t count) noexcept {
  uint32_t n = 0;
  char buf[64];

  for (uint32_t i = 0; i < count; i++) {
    const ImmBits& spec = bits[i];

    uint32_t value = (u8 & uint32_t(spec.mask)) >> spec.shift;
    const char* str = nullptr;

    switch (spec.mode) {
      case ImmBits::kModeLookup:
        str = StringUtils::findPackedString(spec.text, value);
        break;

      case ImmBits::kModeFormat:
        std::snprintf(buf, sizeof(buf), spec.text, unsigned(value));
        str = buf;
        break;

      default:
        return DebugUtils::errored(kErrorInvalidState);
    }

    if (!str[0])
      continue;

    ASMJIT_PROPAGATE(sb.appendChar(++n == 1 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.appendString(str));
  }

  if (n && kImmCharEnd)
    ASMJIT_PROPAGATE(sb.appendChar(kImmCharEnd));

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE static Error X86Logging_formatImmText(StringBuilder& sb, uint32_t u8, uint32_t bits, uint32_t advance, const char* text, uint32_t count = 1) noexcept {
  uint32_t mask = (1U << bits) - 1;
  uint32_t pos = 0;

  for (uint32_t i = 0; i < count; i++, u8 >>= bits, pos += advance) {
    uint32_t value = (u8 & mask) + pos;
    ASMJIT_PROPAGATE(sb.appendChar(i == 0 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.appendString(StringUtils::findPackedString(text, value)));
  }

  if (kImmCharEnd)
    ASMJIT_PROPAGATE(sb.appendChar(kImmCharEnd));

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE static Error X86Logging_explainConst(
  StringBuilder& sb,
  uint32_t logOptions,
  uint32_t instId,
  uint32_t vecSize,
  const Imm& imm) noexcept {

  ASMJIT_UNUSED(logOptions);

  static const char vcmpx[] =
    "EQ_OQ\0" "LT_OS\0"  "LE_OS\0"  "UNORD_Q\0"  "NEQ_UQ\0" "NLT_US\0" "NLE_US\0" "ORD_Q\0"
    "EQ_UQ\0" "NGE_US\0" "NGT_US\0" "FALSE_OQ\0" "NEQ_OQ\0" "GE_OS\0"  "GT_OS\0"  "TRUE_UQ\0"
    "EQ_OS\0" "LT_OQ\0"  "LE_OQ\0"  "UNORD_S\0"  "NEQ_US\0" "NLT_UQ\0" "NLE_UQ\0" "ORD_S\0"
    "EQ_US\0" "NGE_UQ\0" "NGT_UQ\0" "FALSE_OS\0" "NEQ_OS\0" "GE_OQ\0"  "GT_OQ\0"  "TRUE_US\0";

  // Why to make it compatible...
  static const char vpcmpx[] = "EQ\0" "LT\0" "LE\0" "FALSE\0" "NEQ\0" "GE\0"  "GT\0"    "TRUE\0";
  static const char vpcomx[] = "LT\0" "LE\0" "GT\0" "GE\0"    "EQ\0"  "NEQ\0" "FALSE\0" "TRUE\0";

  static const char vshufpd[] = "A0\0A1\0B0\0B1\0A2\0A3\0B2\0B3\0A4\0A5\0B4\0B5\0A6\0A7\0B6\0B7\0";
  static const char vshufps[] = "A0\0A1\0A2\0A3\0A0\0A1\0A2\0A3\0B0\0B1\0B2\0B3\0B0\0B1\0B2\0B3\0";

  static const ImmBits vfpclassxx[] = {
    { 0x07U, 0, ImmBits::kModeLookup, "QNAN\0" "+0\0" "-0\0" "+INF\0" "-INF\0" "DENORMAL\0" "-FINITE\0" "SNAN\0" }
  };

  static const ImmBits vfixupimmxx[] = {
    { 0x01U, 0, ImmBits::kModeLookup, "\0" "+INF_IE\0" },
    { 0x02U, 1, ImmBits::kModeLookup, "\0" "-VE_IE\0"  },
    { 0x04U, 2, ImmBits::kModeLookup, "\0" "-INF_IE\0" },
    { 0x08U, 3, ImmBits::kModeLookup, "\0" "SNAN_IE\0" },
    { 0x10U, 4, ImmBits::kModeLookup, "\0" "ONE_IE\0"  },
    { 0x20U, 5, ImmBits::kModeLookup, "\0" "ONE_ZE\0"  },
    { 0x40U, 6, ImmBits::kModeLookup, "\0" "ZERO_IE\0" },
    { 0x80U, 7, ImmBits::kModeLookup, "\0" "ZERO_ZE\0" }
  };

  static const ImmBits vgetmantxx[] = {
    { 0x03U, 0, ImmBits::kModeLookup, "[1, 2)\0" "[.5, 2)\0" "[.5, 1)\0" "[.75, 1.5)\0" },
    { 0x04U, 2, ImmBits::kModeLookup, "\0" "NO_SIGN\0" },
    { 0x08U, 3, ImmBits::kModeLookup, "\0" "QNAN_IF_SIGN\0" }
  };

  static const ImmBits vmpsadbw[] = {
    { 0x04U, 2, ImmBits::kModeLookup, "BLK1[0]\0" "BLK1[1]\0" },
    { 0x03U, 0, ImmBits::kModeLookup, "BLK2[0]\0" "BLK2[1]\0" "BLK2[2]\0" "BLK2[3]\0" },
    { 0x40U, 6, ImmBits::kModeLookup, "BLK1[4]\0" "BLK1[5]\0" },
    { 0x30U, 4, ImmBits::kModeLookup, "BLK2[4]\0" "BLK2[5]\0" "BLK2[6]\0" "BLK2[7]\0" }
  };

  static const ImmBits vpclmulqdq[] = {
    { 0x01U, 0, ImmBits::kModeLookup, "LQ\0" "HQ\0" },
    { 0x10U, 4, ImmBits::kModeLookup, "LQ\0" "HQ\0" }
  };

  static const ImmBits vperm2x128[] = {
    { 0x0BU, 0, ImmBits::kModeLookup, "A0\0" "A1\0" "B0\0" "B1\0" "\0" "\0" "\0" "\0" "0\0" "0\0" "0\0" "0\0" },
    { 0xB0U, 4, ImmBits::kModeLookup, "A0\0" "A1\0" "B0\0" "B1\0" "\0" "\0" "\0" "\0" "0\0" "0\0" "0\0" "0\0" }
  };

  static const ImmBits vrangexx[] = {
    { 0x03U, 0, ImmBits::kModeLookup, "MIN\0" "MAX\0" "MIN_ABS\0" "MAX_ABS\0" },
    { 0x0CU, 2, ImmBits::kModeLookup, "SIGN_A\0" "SIGN_B\0" "SIGN_0\0" "SIGN_1\0" }
  };

  static const ImmBits vreducexx_vrndscalexx[] = {
    { 0x07U, 0, ImmBits::kModeLookup, "\0" "\0" "\0" "\0" "ROUND\0" "FLOOR\0" "CEIL\0" "TRUNC\0" },
    { 0x08U, 3, ImmBits::kModeLookup, "\0" "SAE\0" },
    { 0xF0U, 4, ImmBits::kModeFormat, "LEN=%d" }
  };

  static const ImmBits vroundxx[] = {
    { 0x07U, 0, ImmBits::kModeLookup, "ROUND\0" "FLOOR\0" "CEIL\0" "TRUNC\0" "\0" "\0" "\0" "\0" },
    { 0x08U, 3, ImmBits::kModeLookup, "\0" "INEXACT\0" }
  };

  uint32_t u8 = imm.getUInt8();
  switch (instId) {
    case X86Inst::kIdVblendpd:
    case X86Inst::kIdBlendpd:
      return X86Logging_formatImmShuf(sb, u8, 1, vecSize / 8);

    case X86Inst::kIdVblendps:
    case X86Inst::kIdBlendps:
      return X86Logging_formatImmShuf(sb, u8, 1, vecSize / 4);

    case X86Inst::kIdVcmppd:
    case X86Inst::kIdVcmpps:
    case X86Inst::kIdVcmpsd:
    case X86Inst::kIdVcmpss:
      return X86Logging_formatImmText(sb, u8, 5, 0, vcmpx);

    case X86Inst::kIdCmppd:
    case X86Inst::kIdCmpps:
    case X86Inst::kIdCmpsd:
    case X86Inst::kIdCmpss:
      return X86Logging_formatImmText(sb, u8, 3, 0, vcmpx);

    case X86Inst::kIdVdbpsadbw:
      return X86Logging_formatImmShuf(sb, u8, 2, 4);

    case X86Inst::kIdVdppd:
    case X86Inst::kIdVdpps:
    case X86Inst::kIdDppd:
    case X86Inst::kIdDpps:
      return X86Logging_formatImmShuf(sb, u8, 1, 8);

    case X86Inst::kIdVmpsadbw:
    case X86Inst::kIdMpsadbw:
      return X86Logging_formatImmBits(sb, u8, vmpsadbw, std::min<uint32_t>(vecSize / 8, 4));

    case X86Inst::kIdVpblendw:
    case X86Inst::kIdPblendw:
      return X86Logging_formatImmShuf(sb, u8, 1, 8);

    case X86Inst::kIdVpblendd:
      return X86Logging_formatImmShuf(sb, u8, 1, std::min<uint32_t>(vecSize / 4, 8));

    case X86Inst::kIdVpclmulqdq:
    case X86Inst::kIdPclmulqdq:
      return X86Logging_formatImmBits(sb, u8, vpclmulqdq, ASMJIT_ARRAY_SIZE(vpclmulqdq));

    case X86Inst::kIdVroundpd:
    case X86Inst::kIdVroundps:
    case X86Inst::kIdVroundsd:
    case X86Inst::kIdVroundss:
    case X86Inst::kIdRoundpd:
    case X86Inst::kIdRoundps:
    case X86Inst::kIdRoundsd:
    case X86Inst::kIdRoundss:
      return X86Logging_formatImmBits(sb, u8, vroundxx, ASMJIT_ARRAY_SIZE(vroundxx));

    case X86Inst::kIdVshufpd:
    case X86Inst::kIdShufpd:
      return X86Logging_formatImmText(sb, u8, 1, 2, vshufpd, std::min<uint32_t>(vecSize / 8, 8));

    case X86Inst::kIdVshufps:
    case X86Inst::kIdShufps:
      return X86Logging_formatImmText(sb, u8, 2, 4, vshufps, 4);

    case X86Inst::kIdVcvtps2ph:
      return X86Logging_formatImmBits(sb, u8, vroundxx, 1);

    case X86Inst::kIdVperm2f128:
    case X86Inst::kIdVperm2i128:
      return X86Logging_formatImmBits(sb, u8, vperm2x128, ASMJIT_ARRAY_SIZE(vperm2x128));

    case X86Inst::kIdVpermilpd:
      return X86Logging_formatImmShuf(sb, u8, 1, vecSize / 8);

    case X86Inst::kIdVpermilps:
      return X86Logging_formatImmShuf(sb, u8, 2, 4);

    case X86Inst::kIdVpshufd:
    case X86Inst::kIdPshufd:
      return X86Logging_formatImmShuf(sb, u8, 2, 4);

    case X86Inst::kIdVpshufhw:
    case X86Inst::kIdVpshuflw:
    case X86Inst::kIdPshufhw:
    case X86Inst::kIdPshuflw:
    case X86Inst::kIdPshufw:
      return X86Logging_formatImmShuf(sb, u8, 2, 4);

    case X86Inst::kIdVfixupimmpd:
    case X86Inst::kIdVfixupimmps:
    case X86Inst::kIdVfixupimmsd:
    case X86Inst::kIdVfixupimmss:
      return X86Logging_formatImmBits(sb, u8, vfixupimmxx, ASMJIT_ARRAY_SIZE(vfixupimmxx));

    case X86Inst::kIdVfpclasspd:
    case X86Inst::kIdVfpclassps:
    case X86Inst::kIdVfpclasssd:
    case X86Inst::kIdVfpclassss:
      return X86Logging_formatImmBits(sb, u8, vfpclassxx, ASMJIT_ARRAY_SIZE(vfpclassxx));

    case X86Inst::kIdVgetmantpd:
    case X86Inst::kIdVgetmantps:
    case X86Inst::kIdVgetmantsd:
    case X86Inst::kIdVgetmantss:
      return X86Logging_formatImmBits(sb, u8, vgetmantxx, ASMJIT_ARRAY_SIZE(vgetmantxx));

    case X86Inst::kIdVpcmpb:
    case X86Inst::kIdVpcmpd:
    case X86Inst::kIdVpcmpq:
    case X86Inst::kIdVpcmpw:
    case X86Inst::kIdVpcmpub:
    case X86Inst::kIdVpcmpud:
    case X86Inst::kIdVpcmpuq:
    case X86Inst::kIdVpcmpuw:
      return X86Logging_formatImmText(sb, u8, 3, 0, vpcmpx);

    case X86Inst::kIdVpcomb:
    case X86Inst::kIdVpcomd:
    case X86Inst::kIdVpcomq:
    case X86Inst::kIdVpcomw:
    case X86Inst::kIdVpcomub:
    case X86Inst::kIdVpcomud:
    case X86Inst::kIdVpcomuq:
    case X86Inst::kIdVpcomuw:
      return X86Logging_formatImmText(sb, u8, 3, 0, vpcomx);

    case X86Inst::kIdVpermq:
    case X86Inst::kIdVpermpd:
      return X86Logging_formatImmShuf(sb, u8, 2, 4);

    case X86Inst::kIdVpternlogd:
    case X86Inst::kIdVpternlogq:
      return X86Logging_formatImmShuf(sb, u8, 1, 8);

    case X86Inst::kIdVrangepd:
    case X86Inst::kIdVrangeps:
    case X86Inst::kIdVrangesd:
    case X86Inst::kIdVrangess:
      return X86Logging_formatImmBits(sb, u8, vrangexx, ASMJIT_ARRAY_SIZE(vrangexx));

    case X86Inst::kIdVreducepd:
    case X86Inst::kIdVreduceps:
    case X86Inst::kIdVreducesd:
    case X86Inst::kIdVreducess:
    case X86Inst::kIdVrndscalepd:
    case X86Inst::kIdVrndscaleps:
    case X86Inst::kIdVrndscalesd:
    case X86Inst::kIdVrndscaless:
      return X86Logging_formatImmBits(sb, u8, vreducexx_vrndscalexx, ASMJIT_ARRAY_SIZE(vreducexx_vrndscalexx));

    case X86Inst::kIdVshuff32x4:
    case X86Inst::kIdVshuff64x2:
    case X86Inst::kIdVshufi32x4:
    case X86Inst::kIdVshufi64x2: {
      uint32_t count = std::max<uint32_t>(vecSize / 16, 2U);
      uint32_t bits = count <= 2 ? 1U : 2U;
      return X86Logging_formatImmShuf(sb, u8, bits, count);
    }

    default:
      return kErrorOk;
  }
}

// ============================================================================
// [asmjit::X86Logging - Format Register]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Logging::formatRegister(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  uint32_t rType,
  uint32_t rId) noexcept {

  ASMJIT_UNUSED(archType);
  const RegFormatInfo& info = x86RegFormatInfo;

#ifndef ASMJIT_DISABLE_COMPILER
  if (Operand::isPackedId(rId)) {
    if (emitter && emitter->getEmitterType() == CodeEmitter::kTypeCompiler) {
      const CodeCompiler* cc = static_cast<const CodeCompiler*>(emitter);
      if (cc->isVirtIdValid(rId)) {
        VirtReg* vReg = cc->getVirtRegById(rId);
        ASMJIT_ASSERT(vReg != nullptr);

        const char* name = vReg->getName();
        if (name && name[0] != '\0')
          ASMJIT_PROPAGATE(sb.appendString(name));
        else
          ASMJIT_PROPAGATE(sb.appendFormat("%%%u", unsigned(Operand::unpackId(rId))));

        if (vReg->getType() != rType && rType <= Reg::kRegMax && (logOptions & Logger::kOptionRegCasts) != 0) {
          const RegFormatInfo::TypeEntry& typeEntry = info.typeEntries[rType];
          if (typeEntry.index)
            ASMJIT_PROPAGATE(sb.appendFormat("@%s", info.typeStrings + typeEntry.index));
        }

        return kErrorOk;
      }
    }
  }
#else
  ASMJIT_UNUSED(logOptions);
#endif

  if (ASMJIT_LIKELY(rType <= Reg::kRegMax)) {
    const RegFormatInfo::NameEntry& nameEntry = info.nameEntries[rType];

    if (rId < nameEntry.specialCount)
      return sb.appendString(info.nameStrings + nameEntry.specialIndex + rId * 4);

    if (rId < nameEntry.count)
      return sb.appendFormat(info.nameStrings + nameEntry.formatIndex, unsigned(rId));

    const RegFormatInfo::TypeEntry& typeEntry = info.typeEntries[rType];
    if (typeEntry.index)
      return sb.appendFormat("%s@%u", info.typeStrings + typeEntry.index, rId);
  }

  return sb.appendFormat("Reg?%u@%u", rType, rId);
}

// ============================================================================
// [asmjit::X86Logging - Format Instruction]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86Logging::formatInstruction(
  StringBuilder& sb,
  uint32_t logOptions,
  const CodeEmitter* emitter,
  uint32_t archType,
  const Inst::Detail& detail, const Operand_* operands, uint32_t count) noexcept {

  uint32_t instId = detail.instId;
  uint32_t options = detail.options;

  // Format instruction options and instruction mnemonic.
  if (instId < X86Inst::_kIdCount) {
    const X86Inst& instInfo = X86Inst::getInst(instId);

    // SHORT|LONG options.
    if (options & X86Inst::kOptionShortForm) ASMJIT_PROPAGATE(sb.appendString("short "));
    if (options & X86Inst::kOptionLongForm) ASMJIT_PROPAGATE(sb.appendString("long "));

    // LOCK|XACQUIRE|XRELEASE options.
    if (options & X86Inst::kOptionXAcquire) ASMJIT_PROPAGATE(sb.appendString("xacquire "));
    if (options & X86Inst::kOptionXRelease) ASMJIT_PROPAGATE(sb.appendString("xrelease "));
    if (options & X86Inst::kOptionLock) ASMJIT_PROPAGATE(sb.appendString("lock "));

    // REP|REPNZ options.
    if (options & (X86Inst::kOptionRep | X86Inst::kOptionRepne)) {
      sb.appendString((options & X86Inst::kOptionRep) ? "rep " : "repnz ");
      if (detail.hasExtraReg()) {
        ASMJIT_PROPAGATE(sb.appendChar('{'));
        ASMJIT_PROPAGATE(formatOperand(sb, logOptions, emitter, archType, detail.extraReg.toReg<Reg>()));
        ASMJIT_PROPAGATE(sb.appendString("} "));
      }
    }

    // REX options.
    if (options & X86Inst::kOptionRex) {
      const uint32_t kRXBWMask = X86Inst::kOptionOpCodeR |
                                 X86Inst::kOptionOpCodeX |
                                 X86Inst::kOptionOpCodeB |
                                 X86Inst::kOptionOpCodeW ;
      if (options & kRXBWMask) {
        sb.appendString("rex.");
        if (options & X86Inst::kOptionOpCodeR) sb.appendChar('r');
        if (options & X86Inst::kOptionOpCodeX) sb.appendChar('x');
        if (options & X86Inst::kOptionOpCodeB) sb.appendChar('b');
        if (options & X86Inst::kOptionOpCodeW) sb.appendChar('w');
        sb.appendChar(' ');
      }
      else {
        ASMJIT_PROPAGATE(sb.appendString("rex "));
      }
    }

    // VEX|EVEX options.
    if (options & X86Inst::kOptionVex3) ASMJIT_PROPAGATE(sb.appendString("vex3 "));
    if (options & X86Inst::kOptionEvex) ASMJIT_PROPAGATE(sb.appendString("evex "));

    ASMJIT_PROPAGATE(sb.appendString(instInfo.getName()));
  }
  else {
    ASMJIT_PROPAGATE(sb.appendFormat("[InstId=#%u]", unsigned(instId)));
  }

  for (uint32_t i = 0; i < count; i++) {
    const Operand_& op = operands[i];
    if (op.isNone()) break;

    ASMJIT_PROPAGATE(sb.appendString(i == 0 ? " " : ", "));
    ASMJIT_PROPAGATE(formatOperand(sb, logOptions, emitter, archType, op));

    if (op.isImm() && (logOptions & Logger::kOptionExplainConsts)) {
      uint32_t vecSize = 16;
      for (uint32_t j = 0; j < count; j++)
        if (operands[j].isReg())
          vecSize = std::max<uint32_t>(vecSize, operands[j].getSize());
      ASMJIT_PROPAGATE(X86Logging_explainConst(sb, logOptions, instId, vecSize, op.as<Imm>()));
    }

    // Support AVX-512 {k}{z}.
    if (i == 0) {
      if (detail.extraReg.getGroup() == X86Reg::kGroupK) {
        ASMJIT_PROPAGATE(sb.appendString(" {"));
        ASMJIT_PROPAGATE(formatOperand(sb, logOptions, emitter, archType, detail.extraReg.toReg<Reg>()));
        ASMJIT_PROPAGATE(sb.appendChar('}'));

        if (options & X86Inst::kOptionZMask)
          ASMJIT_PROPAGATE(sb.appendString("{z}"));
      }
      else if (options & X86Inst::kOptionZMask) {
        ASMJIT_PROPAGATE(sb.appendString(" {z}"));
      }
    }

    // Support AVX-512 {1tox}.
    if (op.isMem() && (options & X86Inst::kOption1ToX))
      ASMJIT_PROPAGATE(sb.appendString(" {1tox}"));
  }

  return kErrorOk;
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
