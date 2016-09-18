// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_LOGGING)

// [Dependencies]
#include "../x86/x86inst.h"
#include "../x86/x86logging.h"
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86Formatter]
// ============================================================================

struct X86FormatterRegData {
  char format[6];
  uint8_t valid;
  uint8_t special;
};

static const X86FormatterRegData x86FormatterRegData[] = {
  { ""     , false, 0 }, // #00 None.
  { ""     , false, 0 }, // #01 Reserved.
  { "rip%u", true , 1 }, // #02 RIP.
  { "seg%u", true , 7 }, // #03 SEG.
  { "r%ub" , true , 8 }, // #04 GPB-LO.
  { "r%uh" , true , 4 }, // #05 GPB-HI.
  { "r%uw" , true , 8 }, // #06 GPW.
  { "r%ud" , true , 8 }, // #07 GPD.
  { "r%u"  , true , 8 }, // #08 GPQ.
  { "fp%u" , true , 0 }, // #09 FP.
  { "mm%u" , true , 0 }, // #10 MM.
  { "k%u"  , true , 0 }, // #11 K.
  { "xmm%u", true , 0 }, // #12 XMM.
  { "ymm%u", true , 0 }, // #13 YMM.
  { "zmm%u", true , 0 }, // #14 ZMM.
  { ""     , false, 0 }, // #15 FUTURE.
  { "bnd%u", true , 0 }, // #16 BND.
  { "cr%u" , true , 0 }, // #17 CR.
  { "dr%u" , true , 0 }  // #18 DR.
};
static const char x86FormatterSegmentNames[] =
  "\0\0\0\0"
  "es:\0"
  "cs:\0"
  "ss:\0"
  "ds:\0"
  "fs:\0"
  "gs:\0"
  "??:\0"; // Unknown 7th segment?

static const char* x86GetAddressSizeString(uint32_t size) noexcept {
  switch (size) {
    case 1 : return "byte ";
    case 2 : return "word ";
    case 4 : return "dword ";
    case 8 : return "qword ";
    case 10: return "tword ";
    case 16: return "oword ";
    case 32: return "yword ";
    case 64: return "zword ";
    default: return "";
  }
}

X86Formatter::X86Formatter() noexcept {}
X86Formatter::~X86Formatter() noexcept {}

ASMJIT_FAVOR_SIZE Error X86Formatter::formatRegister(StringBuilder& out, uint32_t logOptions, uint32_t regType, uint32_t regId) const noexcept {
  static const char reg8l[] = "al\0\0" "cl\0\0" "dl\0\0" "bl\0\0" "spl\0"  "bpl\0"  "sil\0"  "dil\0" ;
  static const char reg8h[] = "ah\0\0" "ch\0\0" "dh\0\0" "bh\0\0" "--\0\0" "--\0\0" "--\0\0" "--\0\0";
  static const char reg32[] = "eax\0"  "ecx\0"  "edx\0"  "ebx\0"  "esp\0"  "ebp\0"  "esi\0"  "edi\0" ;
  static const char reg64[] = "rax\0"  "rcx\0"  "rdx\0"  "rbx\0"  "rsp\0"  "rbp\0"  "rsi\0"  "rdi\0" ;

  if (regType < ASMJIT_ARRAY_SIZE(x86FormatterRegData)) {
    const X86FormatterRegData& rfd = x86FormatterRegData[regType];
    if (rfd.valid) {
      if (regId < rfd.special) {
        const char prefix = '\0';
        const char* s = nullptr;
        size_t len = kInvalidIndex;

        if (regType == X86Reg::kRegGpbLo) {
          s = reg8l;
        }
        else if (regType == X86Reg::kRegGpbHi) {
          s = reg8h;
        }
        else if (regType == X86Reg::kRegGpw) {
          s = reg32 + 1;
        }
        else if (regType == X86Reg::kRegGpd) {
          s = reg32;
        }
        else if (regType == X86Reg::kRegGpq) {
          s = reg64;
        }
        else if (regType == X86Reg::kRegRip) {
          s = "rip";
        }
        else {
          if (regId == 0) goto Invalid;
          s = x86FormatterSegmentNames;
          len = 2;
        }

        out.appendString(s + regId * 4, len);
        return kErrorOk;
      }

      out.appendFormat(rfd.format, regId);
      return kErrorOk;
    }
  }

Invalid:
  out.appendFormat("InvalidReg[Type=%u ID=%u]", regType, regId);
  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86Formatter::formatOperand(StringBuilder& out, uint32_t logOptions, const Operand_& op) const noexcept {
  if (op.isReg()) {
    const Reg& r = static_cast<const Reg&>(op);
    if (r.isPhysReg()) {
      formatRegister(out, logOptions, r.getRegType(), r.getId());
      return kErrorOk;
    }
    else {
      if (hasVirtRegHandler())
        return formatVirtReg(out, logOptions, r);

      out.appendFormat("VirtReg[Type=%u ID=%u]", r.getRegType(), r.getId());
      return kErrorOk;
    }
  }

  if (op.isMem()) {
    const X86Mem& m = static_cast<const X86Mem&>(op);
    out.appendString(x86GetAddressSizeString(m.getSize()));

    // Segment override prefix.
    uint32_t seg = m.getSegmentId();
    if (seg != X86Seg::kIdNone && seg < X86Seg::kIdCount)
      out.appendString(x86FormatterSegmentNames + seg * 4);

    out.appendChar('[');
    if (m.isAbs())
      out.appendString("abs ");

    if (m.hasBase()) {
      if (m.hasBaseLabel()) {
        out.appendFormat("L%u", Operand::unpackId(m.getBaseId()));
      }
      else {
        X86Reg baseReg = X86Reg::fromTypeAndId(m.getBaseType(), m.getBaseId());
        if (m.isArgHome()) out.appendString("$");
        if (m.isRegHome()) out.appendString("&");
        formatOperand(out, logOptions, baseReg);
      }
    }

    if (m.hasIndex()) {
      X86Reg indexReg = X86Reg::fromTypeAndId(m.getIndexType(), m.getIndexId());
      out.appendChar('+');

      formatOperand(out, logOptions, indexReg);
      if (m.hasShift())
        out.appendFormat("*%u", 1 << m.getShift());
    }

    uint64_t off = static_cast<uint64_t>(m.getOffset());
    if (off) {
      uint32_t base = 10;
      char prefix = '+';

      if (static_cast<int64_t>(off) < 0) {
        off = ~off + 1;
        prefix = '-';
      }

      out.appendChar(prefix);
      if ((logOptions & Logger::kOptionHexDisplacement) != 0 && off > 9) {
        out.appendString("0x", 2);
        base = 16;
      }
      out.appendUInt(off, base);
    }

    out.appendChar(']');
    return kErrorOk;
  }

  if (op.isImm()) {
    const Imm& i = static_cast<const Imm&>(op);
    int64_t val = i.getInt64();

    if ((logOptions & Logger::kOptionHexImmediate) != 0 && static_cast<uint64_t>(val) > 9)
      out.appendUInt(static_cast<uint64_t>(val), 16);
    else
      out.appendInt(val, 10);
    return kErrorOk;
  }

  if (op.isLabel()) {
    out.appendFormat("L%u", Operand::unpackId(op.getId()));
    return kErrorOk;
  }

  out.appendString("None", 4);
  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error X86Formatter::formatInstruction(
  StringBuilder& out,
  uint32_t logOptions,
  uint32_t instId,
  uint32_t options,
  const Operand_& opExtra,
  const Operand_* opArray, uint32_t opCount) const noexcept {

  bool opExtraDone = false;

  // Format instruction options and instruction mnemonic.
  if (instId < X86Inst::_kIdCount) {
    const X86Inst& instInfo = X86Inst::getInst(instId);

    // SHORT/LONG forms.
    if (options & X86Inst::kOptionShortForm) out.appendString("short ");
    if (options & X86Inst::kOptionLongForm) out.appendString("long ");

    // LOCK option.
    if (options & X86Inst::kOptionLock) out.appendString("lock ");

    // REP options.
    if (options & (X86Inst::kOptionRep | X86Inst::kOptionRepnz)) {
      const char* rep = "repnz ";
      if ((options & (X86Inst::kOptionRep | X86Inst::kOptionRepnz)) == X86Inst::kOptionRep)
        rep = instInfo.hasFlag(X86Inst::kInstFlagRepnz) ? "repz " : "rep ";

      out.appendString(rep);
      if (!opExtra.isNone()) {
        out.appendChar('{');
        formatOperand(out, logOptions, opExtra);
        out.appendString("} ");
        opExtraDone = true;
      }
    }

    // REX options.
    if (options & X86Inst::kOptionRex) {
      const uint32_t kRXBWMask = X86Inst::kOptionOpCodeR |
                                 X86Inst::kOptionOpCodeX |
                                 X86Inst::kOptionOpCodeB |
                                 X86Inst::kOptionOpCodeW ;
      if (options & kRXBWMask) {
        out.appendString("rex.");
        if (options & X86Inst::kOptionOpCodeR) out.appendChar('r');
        if (options & X86Inst::kOptionOpCodeX) out.appendChar('x');
        if (options & X86Inst::kOptionOpCodeB) out.appendChar('b');
        if (options & X86Inst::kOptionOpCodeW) out.appendChar('w');
        out.appendChar(' ');
      }
      else {
        out.appendString("rex ");
      }
    }

    // VEX options.
    if (options & X86Inst::kOptionVex3) {
      out.appendString("vex3 ");
    }

    out.appendString(instInfo.getName());
  }
  else {
    out.appendFormat("<unknown id=#%u>", static_cast<unsigned int>(instId));
  }

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand_& op = opArray[i];
    if (op.isNone()) break;

    out.appendString(i == 0 ? " " : ", ");
    formatOperand(out, logOptions, op);

    // Support AVX-512 {k}{z}.
    if (i == 0) {
      const uint32_t kExtMsk =
        X86Inst::kOptionOpExtra |
        X86Inst::kOptionRep     |
        X86Inst::kOptionRepnz   ;

      if ((options & kExtMsk) == X86Inst::kOptionOpExtra) {
        out.appendString(" {");
        formatOperand(out, logOptions, opExtra);
        out.appendChar('}');

        if (options & X86Inst::kOptionKZ)
          out.appendString("{z}");
      }
      else if (options & X86Inst::kOptionKZ) {
        out.appendString(" {z}");
      }
    }

    // Support AVX-512 {1tox}.
    if (op.isMem() && (options & X86Inst::kOption1ToX)) {
      out.appendString(" {1tox}");
    }
  }

  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
