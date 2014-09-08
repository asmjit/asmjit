// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../base/intutil.h"
#include "../base/logger.h"
#include "../base/runtime.h"
#include "../base/string.h"
#include "../base/vmem.h"
#include "../x86/x86assembler.h"
#include "../x86/x86cpuinfo.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Constants]
// ============================================================================

enum { kRexShift = 6 };
enum { kRexForbidden = 0x80 };
enum { kMaxCommentLength = 80 };

// 2-byte VEX prefix.
//   [0] kVex2Byte.
//   [1] RvvvvLpp.
enum { kVex2Byte = 0xC5 };

// 3-byte VEX prefix.
//   [0] kVex3Byte.
//   [1] RXBmmmmm.
//   [2] WvvvvLpp.
enum { kVex3Byte = 0xC4 };

// 3-byte XOP prefix.
//   [0] kXopByte
//   [1] RXBmmmmm
//   [2] WvvvvLpp
enum { kXopByte = 0x8F };

// AsmJit specific (used to encode VVVV field in XOP/VEX).
enum kVexVVVV {
  kVexVVVVShift = 12,
  kVexVVVVMask = 0xF << kVexVVVVShift
};

//! \internal
//!
//! Instruction 2-byte/3-byte opcode prefix definition.
struct X86OpCodeMM {
  uint8_t len;
  uint8_t data[3];
};

//! \internal
//!
//! Mandatory prefixes encoded in 'asmjit' opcode [66, F3, F2] and asmjit
//! extensions
static const uint8_t x86OpCodePP[8] = {
  0x00,
  0x66,
  0xF3,
  0xF2,
  0x00,
  0x00,
  0x00,
  0x9B
};

//! \internal
//!
//! Instruction 2-byte/3-byte opcode prefix data.
static const X86OpCodeMM x86OpCodeMM[] = {
  { 0, { 0x00, 0x00, 0 } },
  { 1, { 0x0F, 0x00, 0 } },
  { 2, { 0x0F, 0x38, 0 } },
  { 2, { 0x0F, 0x3A, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 0, { 0x00, 0x00, 0 } },
  { 2, { 0x0F, 0x01, 0 } }
};

static const uint8_t x86SegmentPrefix[8] = { 0x00, 0x26, 0x2E, 0x36, 0x3E, 0x64, 0x65 };
static const uint8_t x86OpCodePushSeg[8] = { 0x00, 0x06, 0x0E, 0x16, 0x1E, 0xA0, 0xA8 };
static const uint8_t x86OpCodePopSeg[8]  = { 0x00, 0x07, 0x00, 0x17, 0x1F, 0xA1, 0xA9 };

// ============================================================================
// [Utils]
// ============================================================================

//! Encode MODR/M.
static ASMJIT_INLINE uint32_t x86EncodeMod(uint32_t m, uint32_t o, uint32_t rm) {
  return (m << 6) + (o << 3) + rm;
}

//! Encode SIB.
static ASMJIT_INLINE uint32_t x86EncodeSib(uint32_t s, uint32_t i, uint32_t b) {
  return (s << 6) + (i << 3) + b;
}

//! Get whether the two pointers `a` and `b` can be encoded by using relative
//! displacement, which fits into a signed 32-bit integer.
static ASMJIT_INLINE bool x64IsRelative(Ptr a, Ptr b) {
  SignedPtr diff = static_cast<SignedPtr>(a) - static_cast<SignedPtr>(b);
  return IntUtil::isInt32(diff);
}

// ============================================================================
// [Macros]
// ============================================================================

#define ENC_OPS(_Op0_, _Op1_, _Op2_) \
  ((kOperandType##_Op0_) + ((kOperandType##_Op1_) << 3) + ((kOperandType##_Op2_) << 6))

#define ADD_66H_P(_Exp_) \
  do { \
    opCode |= (static_cast<uint32_t>(_Exp_) << kX86InstOpCode_PP_Shift); \
  } while (0)

#define ADD_66H_P_BY_SIZE(_Size_) \
  do { \
    opCode |= (static_cast<uint32_t>(_Size_) & 0x02) << (kX86InstOpCode_PP_Shift - 1); \
  } while (0)

#define ADD_REX_W(_Exp_) \
  do { \
    if (Arch == kArchX64) \
      opX |= static_cast<uint32_t>(_Exp_) << 3; \
  } while (0)

#define ADD_REX_W_BY_SIZE(_Size_) \
  do { \
    if (Arch == kArchX64) \
      opX |= static_cast<uint32_t>(_Size_) & 0x08; \
  } while (0)

#define ADD_REX_B(_Reg_) \
  do { \
    if (Arch == kArchX64) \
      opX |= static_cast<uint32_t>(_Reg_) >> 3; \
  } while (0)

#define ADD_VEX_W(_Exp_) \
  do { \
    opX |= static_cast<uint32_t>(_Exp_) << 3; \
  } while (0)

#define ADD_VEX_L(_Exp_) \
  do { \
    opCode |= static_cast<uint32_t>(_Exp_) << kX86InstOpCode_L_Shift; \
  } while (0)

#define EMIT_BYTE(_Val_) \
  do { \
    cursor[0] = static_cast<uint8_t>(_Val_); \
    cursor += 1; \
  } while (0)

#define EMIT_WORD(_Val_) \
  do { \
    reinterpret_cast<uint16_t*>(cursor)[0] = static_cast<uint16_t>(_Val_); \
    cursor += 2; \
  } while (0)

#define EMIT_DWORD(_Val_) \
  do { \
    reinterpret_cast<uint32_t*>(cursor)[0] = static_cast<uint32_t>(_Val_); \
    cursor += 4; \
  } while (0)

#define EMIT_QWORD(_Val_) \
  do { \
    reinterpret_cast<uint64_t*>(cursor)[0] = static_cast<uint64_t>(_Val_); \
    cursor += 8; \
  } while (0)

#define EMIT_OP(_Val_) \
  do { \
    EMIT_BYTE((_Val_) & 0xFF); \
  } while (0)

#define EMIT_PP(_Val_) \
  do { \
    uint32_t ppIndex = ((_Val_) >> kX86InstOpCode_PP_Shift) & (kX86InstOpCode_PP_Mask >> kX86InstOpCode_PP_Shift); \
    uint8_t ppCode = x86OpCodePP[ppIndex]; \
    \
    if (!ppIndex) \
      break; \
    \
    cursor[0] = ppCode; \
    cursor++; \
  } while (0)

#define EMIT_MM(_Val_) \
  do { \
    uint32_t mmIndex = ((_Val_) >> kX86InstOpCode_MM_Shift) & (kX86InstOpCode_MM_Mask >> kX86InstOpCode_MM_Shift); \
    const X86OpCodeMM& mmCode = x86OpCodeMM[mmIndex]; \
    \
    if (!mmIndex) \
      break; \
    \
    cursor[0] = mmCode.data[0]; \
    cursor[1] = mmCode.data[1]; \
    cursor += mmCode.len; \
  } while (0)

// ============================================================================
// [asmjit::X86Assembler - Construction / Destruction]
// ============================================================================

X86Assembler::X86Assembler(Runtime* runtime, uint32_t arch) :
  Assembler(runtime),
  zax(NoInit),
  zcx(NoInit),
  zdx(NoInit),
  zbx(NoInit),
  zsp(NoInit),
  zbp(NoInit),
  zsi(NoInit),
  zdi(NoInit) {

  setArch(arch);
}

X86Assembler::~X86Assembler() {}

// ============================================================================
// [asmjit::X86Assembler - Arch]
// ============================================================================

Error X86Assembler::setArch(uint32_t arch) {
#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86) {
    _arch = kArchX86;
    _regSize = 4;

    _regCount.reset();
    _regCount._gp = 8;
    _regCount._fp = 8;
    _regCount._mm = 8;
    _regCount._xy = 8;

    zax = x86::eax;
    zcx = x86::ecx;
    zdx = x86::edx;
    zbx = x86::ebx;
    zsp = x86::esp;
    zbp = x86::ebp;
    zsi = x86::esi;
    zdi = x86::edi;

    return kErrorOk;
  }
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64) {
    _arch = kArchX64;
    _regSize = 8;

    _regCount.reset();
    _regCount._gp = 16;
    _regCount._fp = 8;
    _regCount._mm = 8;
    _regCount._xy = 16;

    zax = x86::rax;
    zcx = x86::rcx;
    zdx = x86::rdx;
    zbx = x86::rbx;
    zsp = x86::rsp;
    zbp = x86::rbp;
    zsi = x86::rsi;
    zdi = x86::rdi;

    return kErrorOk;
  }
#endif // ASMJIT_BUILD_X64

  ASMJIT_ASSERT(!"Reached");
  return kErrorInvalidArgument;
}

// ============================================================================
// [asmjit::X86Assembler - Embed]
// ============================================================================

Error X86Assembler::embedLabel(const Label& op) {
  ASMJIT_ASSERT(op.getId() != kInvalidValue);
  uint32_t regSize = _regSize;

  if (getRemainingSpace() < regSize)
    ASMJIT_PROPAGATE_ERROR(_grow(regSize));

  uint8_t* cursor = getCursor();

  LabelData* label = getLabelData(op.getId());
  RelocData reloc;

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger)
    _logger->logFormat(kLoggerStyleData, regSize == 4 ? ".dd L%u\n" : ".dq L%u\n", op.getId());
#endif // !ASMJIT_DISABLE_LOGGER

  reloc.type = kRelocRelToAbs;
  reloc.size = regSize;
  reloc.from = static_cast<Ptr>(getOffset());
  reloc.data = 0;

  if (label->offset != -1) {
    // Bound label.
    reloc.data = static_cast<Ptr>(static_cast<SignedPtr>(label->offset));
  }
  else {
    // Non-bound label. Need to chain.
    LabelLink* link = _newLabelLink();

    link->prev = (LabelLink*)label->links;
    link->offset = getOffset();
    link->displacement = 0;
    link->relocId = _relocList.getLength();

    label->links = link;
  }

  if (_relocList.append(reloc) != kErrorOk)
    return setError(kErrorNoHeapMemory);

  // Emit dummy intptr_t (4 or 8 bytes; depends on the address size).
  if (regSize == 4)
    EMIT_DWORD(0);
  else
    EMIT_QWORD(0);

  setCursor(cursor);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Assembler - Align]
// ============================================================================

Error X86Assembler::align(uint32_t mode, uint32_t offset) {
#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger)
    _logger->logFormat(kLoggerStyleDirective,
      "%s.align %u\n", _logger->getIndentation(), static_cast<unsigned int>(offset));
#endif // !ASMJIT_DISABLE_LOGGER

  if (offset <= 1 || !IntUtil::isPowerOf2(offset) || offset > 64)
    return setError(kErrorInvalidArgument);

  uint32_t i = static_cast<uint32_t>(IntUtil::deltaTo<size_t>(getOffset(), offset));
  if (i == 0)
    return kErrorOk;

  if (getRemainingSpace() < i)
    ASMJIT_PROPAGATE_ERROR(_grow(i));

  uint8_t* cursor = getCursor();
  uint8_t alignPattern = 0xCC;

  if (mode == kAlignCode) {
    alignPattern = 0x90;

    if (hasFeature(kCodeGenOptimizedAlign)) {
      const X86CpuInfo* cpuInfo = static_cast<const X86CpuInfo*>(getRuntime()->getCpuInfo());

      // NOPs optimized for Intel:
      //   Intel 64 and IA-32 Architectures Software Developer's Manual
      //   - Volume 2B
      //   - Instruction Set Reference N-Z
      //     - NOP

      // NOPs optimized for AMD:
      //   Software Optimization Guide for AMD Family 10h Processors (Quad-Core)
      //   - 4.13 - Code Padding with Operand-Size Override and Multibyte NOP

      // Intel and AMD.
      static const uint8_t nop1[] = { 0x90 };
      static const uint8_t nop2[] = { 0x66, 0x90 };
      static const uint8_t nop3[] = { 0x0F, 0x1F, 0x00 };
      static const uint8_t nop4[] = { 0x0F, 0x1F, 0x40, 0x00 };
      static const uint8_t nop5[] = { 0x0F, 0x1F, 0x44, 0x00, 0x00 };
      static const uint8_t nop6[] = { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 };
      static const uint8_t nop7[] = { 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 };
      static const uint8_t nop8[] = { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
      static const uint8_t nop9[] = { 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };

      // AMD.
      static const uint8_t nop10[] = { 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
      static const uint8_t nop11[] = { 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };

      const uint8_t* p;
      uint32_t n;

      if (cpuInfo->getVendorId() == kCpuVendorIntel && (
          (cpuInfo->getFamily() & 0x0F) == 0x06 ||
          (cpuInfo->getFamily() & 0x0F) == 0x0F)) {
        do {
          switch (i) {
            case  1: p = nop1; n = 1; break;
            case  2: p = nop2; n = 2; break;
            case  3: p = nop3; n = 3; break;
            case  4: p = nop4; n = 4; break;
            case  5: p = nop5; n = 5; break;
            case  6: p = nop6; n = 6; break;
            case  7: p = nop7; n = 7; break;
            case  8: p = nop8; n = 8; break;
            default: p = nop9; n = 9; break;
          }

          i -= n;
          do {
            EMIT_BYTE(*p++);
          } while (--n);
        } while (i);
      }
      else if (cpuInfo->getVendorId() == kCpuVendorAmd && cpuInfo->getFamily() >= 0x0F) {
        do {
          switch (i) {
            case  1: p = nop1 ; n =  1; break;
            case  2: p = nop2 ; n =  2; break;
            case  3: p = nop3 ; n =  3; break;
            case  4: p = nop4 ; n =  4; break;
            case  5: p = nop5 ; n =  5; break;
            case  6: p = nop6 ; n =  6; break;
            case  7: p = nop7 ; n =  7; break;
            case  8: p = nop8 ; n =  8; break;
            case  9: p = nop9 ; n =  9; break;
            case 10: p = nop10; n = 10; break;
            default: p = nop11; n = 11; break;
          }

          i -= n;
          do {
            EMIT_BYTE(*p++);
          } while (--n);
        } while (i);
      }
    }
  }

  while (i) {
    EMIT_BYTE(alignPattern);
    i--;
  }

  setCursor(cursor);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Assembler - Reloc]
// ============================================================================

size_t X86Assembler::_relocCode(void* _dst, Ptr baseAddress) const {
  uint32_t arch = getArch();
  uint8_t* dst = static_cast<uint8_t*>(_dst);

#if !defined(ASMJIT_DISABLE_LOGGER)
  Logger* logger = getLogger();
#endif // ASMJIT_DISABLE_LOGGER

  size_t minCodeSize = getOffset();   // Current offset is the minimum code size.
  size_t maxCodeSize = getCodeSize(); // Includes all possible trampolines.

  // We will copy the exact size of the generated code. Extra code for trampolines
  // is generated on-the-fly by the relocator (this code doesn't exist at the moment).
  ::memcpy(dst, _buffer, minCodeSize);

  // Trampoline pointer.
  uint8_t* tramp = dst + minCodeSize;

  // Relocate all recorded locations.
  size_t relocCount = _relocList.getLength();
  const RelocData* relocData = _relocList.getData();

  for (size_t i = 0; i < relocCount; i++) {
    const RelocData& r = relocData[i];

    // Make sure that the `RelocData` is correct.
    Ptr ptr = r.data;

    size_t offset = static_cast<size_t>(r.from);
    ASMJIT_ASSERT(offset + r.size <= static_cast<Ptr>(maxCodeSize));

    // Whether to use trampoline, can be only used if relocation type is
    // kRelocAbsToRel on 64-bit.
    bool useTrampoline = false;

    switch (r.type) {
      case kRelocAbsToAbs:
        break;

      case kRelocRelToAbs:
        ptr += baseAddress;
        break;

      case kRelocAbsToRel:
        ptr -= baseAddress + r.from + 4;
        break;

      case kRelocTrampoline:
        ptr -= baseAddress + r.from + 4;
        if (!IntUtil::isInt32(static_cast<SignedPtr>(ptr))) {
          ptr = (Ptr)tramp - (baseAddress + r.from + 4);
          useTrampoline = true;
        }
        break;

      default:
        ASMJIT_ASSERT(!"Reached");
    }

    switch (r.size) {
      case 8:
        *reinterpret_cast<int64_t*>(dst + offset) = static_cast<int64_t>(ptr);
        break;

      case 4:
        *reinterpret_cast<int32_t*>(dst + offset) = static_cast<int32_t>(static_cast<SignedPtr>(ptr));
        break;

      default:
        ASMJIT_ASSERT(!"Reached");
    }

    // Handle the case where trampoline has been used.
    if (useTrampoline) {
      // Bytes that replace [REX, OPCODE] bytes.
      uint32_t byte0 = 0xFF;
      uint32_t byte1 = dst[offset - 1];

      // Call, patch to FF/2 (-> 0x15).
      if (byte1 == 0xE8)
        byte1 = x86EncodeMod(0, 2, 5);
      // Jmp, patch to FF/4 (-> 0x25).
      else if (byte1 == 0xE9)
        byte1 = x86EncodeMod(0, 4, 5);

      // Patch `jmp/call` instruction.
      ASMJIT_ASSERT(offset >= 2);
      dst[offset - 2] = byte0;
      dst[offset - 1] = byte1;

      // Absolute address.
      ((uint64_t*)tramp)[0] = static_cast<uint64_t>(r.data);

      // Advance trampoline pointer.
      tramp += 8;

#if !defined(ASMJIT_DISABLE_LOGGER)
      if (logger)
        logger->logFormat(kLoggerStyleComment, "; Trampoline %llX\n", r.data);
#endif // !ASMJIT_DISABLE_LOGGER
    }
  }

  if (arch == kArchX64)
    return (size_t)(tramp - dst);
  else
    return (size_t)(minCodeSize);
}

// ============================================================================
// [asmjit::X86Assembler - Logging]
// ============================================================================

#if !defined(ASMJIT_DISABLE_LOGGER)
// Logging helpers.
static const char* AssemblerX86_operandSize[] = {
  "",
  "byte ptr ",
  "word ptr ",
  NULL,
  "dword ptr ",
  NULL,
  NULL,
  NULL,
  "qword ptr ",
  NULL,
  "tword ptr ",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  "oword ptr "
};

static const char X86Assembler_segName[] =
  "\0\0\0\0"
  "es:\0"
  "cs:\0"
  "ss:\0"
  "ds:\0"
  "fs:\0"
  "gs:\0"
  "\0\0\0\0";

static void X86Assembler_dumpRegister(StringBuilder& sb, uint32_t type, uint32_t index) {
  // -- (Not-Encodable).
  static const char reg8l[] = "al\0\0" "cl\0\0" "dl\0\0" "bl\0\0" "spl\0"  "bpl\0"  "sil\0"  "dil\0" ;
  static const char reg8h[] = "ah\0\0" "ch\0\0" "dh\0\0" "bh\0\0" "--\0\0" "--\0\0" "--\0\0" "--\0\0";
  static const char reg16[] = "ax\0\0" "cx\0\0" "dx\0\0" "bx\0\0" "sp\0\0" "bp\0\0" "si\0\0" "di\0\0";

  char suffix = '\0';

  switch (type) {
    case kX86RegTypeGpbLo:
      if (index >= 8) {
        sb._appendChar('r');
        suffix = 'b';
        goto _EmitID;
      }

      sb._appendString(&reg8l[index * 4]);
      return;

    case _kX86RegTypePatchedGpbHi:
      if (index < 4)
        goto _EmitNE;

      index -= 4;
      // ... Fall through ...

    case kX86RegTypeGpbHi:
      if (index >= 4)
        goto _EmitNE;

      sb._appendString(&reg8h[index * 4]);
      return;

_EmitNE:
      sb._appendString("--", 2);
      return;

    case kX86RegTypeGpw:
      if (index >= 8) {
        sb._appendChar('r');
        suffix = 'w';
        goto _EmitID;
      }

      sb._appendString(&reg16[index * 4]);
      return;

    case kX86RegTypeGpd:
      if (index >= 8) {
        sb._appendChar('r');
        suffix = 'd';
        goto _EmitID;
      }

      sb._appendChar('e');
      sb._appendString(&reg16[index * 4]);
      return;

    case kX86RegTypeGpq:
      sb._appendChar('r');
      if (index >= 8)
        goto _EmitID;

      sb._appendString(&reg16[index * 4]);
      return;

    case kX86RegTypeFp:
      sb._appendString("fp", 2);
      goto _EmitID;

    case kX86RegTypeMm:
      sb._appendString("mm", 2);
      goto _EmitID;

    case kX86RegTypeXmm:
      sb._appendString("xmm", 3);
      goto _EmitID;

    case kX86RegTypeYmm:
      sb._appendString("ymm", 3);
      goto _EmitID;

    case kX86RegTypeSeg:
      if (index >= kX86SegCount)
        goto _EmitNE;

      sb._appendString(&X86Assembler_segName[index * 4], 2);
      return;

    default:
      return;
  }

_EmitID:
  sb._appendUInt32(index);

  if (suffix)
    sb._appendChar(suffix);
}

static void X86Assembler_dumpOperand(StringBuilder& sb, uint32_t arch, const Operand* op, uint32_t loggerOptions) {
  if (op->isReg()) {
    X86Assembler_dumpRegister(sb,
      static_cast<const X86Reg*>(op)->getRegType(),
      static_cast<const X86Reg*>(op)->getRegIndex());
  }
  else if (op->isMem()) {
    const X86Mem* m = static_cast<const X86Mem*>(op);

    uint32_t type = kX86RegTypeGpd;
    uint32_t seg = m->getSegment();
    bool isAbsolute = false;

    if (arch == kArchX86) {
      if (!m->hasGpdBase())
        type = kX86RegTypeGpw;
    }
    else {
      if (!m->hasGpdBase())
        type = kX86RegTypeGpq;
    }

    if (op->getSize() <= 16)
      sb._appendString(AssemblerX86_operandSize[op->getSize()]);

    if (seg < kX86SegCount)
      sb._appendString(&X86Assembler_segName[seg * 4]);

    sb._appendChar('[');
    switch (m->getMemType()) {
      case kMemTypeBaseIndex:
      case kMemTypeStackIndex:
        // [base + index << shift + displacement]
        X86Assembler_dumpRegister(sb, type, m->getBase());
        break;

      case kMemTypeLabel:
        // [label + index << shift + displacement]
        sb.appendFormat("L%u", m->getBase());
        break;

      case kMemTypeAbsolute:
        // [absolute]
        isAbsolute = true;
        sb.appendUInt(static_cast<uint32_t>(m->getDisplacement()), 16);
        break;
    }

    if (m->hasIndex()) {
      switch (m->getVSib()) {
        case kX86MemVSibXmm: type = kX86RegTypeXmm; break;
        case kX86MemVSibYmm: type = kX86RegTypeYmm; break;
      }

      sb._appendChar('+');
      X86Assembler_dumpRegister(sb, type, m->getIndex());

      if (m->getShift()) {
        sb._appendChar('*');
        sb._appendChar("1248"[m->getShift() & 3]);
      }
    }

    if (m->getDisplacement() && !isAbsolute) {
      uint32_t base = 10;
      int32_t dispOffset = m->getDisplacement();

      char prefix = '+';
      if (dispOffset < 0) {
        dispOffset = -dispOffset;
        prefix = '-';
      }

      sb._appendChar(prefix);
      if ((loggerOptions & (1 << kLoggerOptionHexDisplacement)) != 0 && dispOffset > 9) {
        sb._appendString("0x", 2);
        base = 16;
      }
      sb.appendUInt(static_cast<uint32_t>(dispOffset), base);
    }

    sb._appendChar(']');
  }
  else if (op->isImm()) {
    const Imm* i = static_cast<const Imm*>(op);
    int64_t val = i->getInt64();

    if ((loggerOptions & (1 << kLoggerOptionHexImmediate)) && static_cast<uint64_t>(val) > 9)
      sb.appendUInt(static_cast<uint64_t>(val), 16);
    else
      sb.appendInt(val, 10);
  }
  else if (op->isLabel()) {
    sb.appendFormat("L%u", op->getId());
  }
  else {
    sb._appendString("None", 4);
  }
}

static bool X86Assembler_dumpInstruction(StringBuilder& sb,
  uint32_t arch,
  uint32_t code, uint32_t options,
  const Operand* o0,
  const Operand* o1,
  const Operand* o2,
  const Operand* o3,
  uint32_t loggerOptions) {

  if (!sb.reserve(sb.getLength() + 128))
    return false;

  // Rex, lock and short prefix.
  if (options & kX86InstOptionRex)
    sb._appendString("rex ", 4);

  if (options & kX86InstOptionLock)
    sb._appendString("lock ", 5);

  if (options & kInstOptionShortForm)
    sb._appendString("short ", 6);

  // Dump instruction name.
  sb._appendString(_x86InstInfo[code].getInstName());

  // Dump operands.
  if (!o0->isNone()) {
    sb._appendChar(' ');
    X86Assembler_dumpOperand(sb, arch, o0, loggerOptions);
  }

  if (!o1->isNone()) {
    sb._appendString(", ", 2);
    X86Assembler_dumpOperand(sb, arch, o1, loggerOptions);
  }

  if (!o2->isNone()) {
    sb._appendString(", ", 2);
    X86Assembler_dumpOperand(sb, arch, o2, loggerOptions);
  }

  if (!o3->isNone()) {
    sb._appendString(", ", 2);
    X86Assembler_dumpOperand(sb, arch, o3, loggerOptions);
  }

  return true;
}

static bool X86Assembler_dumpComment(StringBuilder& sb, size_t len, const uint8_t* binData, size_t binLength, size_t dispSize, const char* comment) {
  size_t currentLength = len;
  size_t commentLength = comment ? StringUtil::nlen(comment, kMaxCommentLength) : 0;

  ASMJIT_ASSERT(binLength >= dispSize);

  if (binLength || commentLength) {
    size_t align = 36;
    char sep = ';';

    for (size_t i = (binLength == 0); i < 2; i++) {
      size_t begin = sb.getLength();

      // Append align.
      if (currentLength < align) {
        if (!sb.appendChars(' ', align - currentLength))
          return false;
      }

      // Append separator.
      if (sep) {
        if (!(sb.appendChar(sep) & sb.appendChar(' ')))
          return false;
      }

      // Append binary data or comment.
      if (i == 0) {
        if (!sb.appendHex(binData, binLength - dispSize))
          return false;
        if (!sb.appendChars('.', dispSize * 2))
          return false;
        if (commentLength == 0)
          break;
      }
      else {
        if (!sb.appendString(comment, commentLength))
          return false;
      }

      currentLength += sb.getLength() - begin;
      align += 22;
      sep = '|';
    }
  }

  return sb.appendChar('\n');
}
#endif // !ASMJIT_DISABLE_LOGGER

// ============================================================================
// [asmjit::X86Assembler - Emit]
// ============================================================================

//! \internal
static const Operand::VRegOp x86PatchedHiRegs[4] = {
  // --------------+---+--------------------------------+--------------+------+
  // Operand       | S | Register Code                  | OperandId    |Unused|
  // --------------+---+--------------------------------+--------------+------+
  { kOperandTypeReg, 1 , (_kX86RegTypePatchedGpbHi << 8) | 4, kInvalidValue, 0, 0 },
  { kOperandTypeReg, 1 , (_kX86RegTypePatchedGpbHi << 8) | 5, kInvalidValue, 0, 0 },
  { kOperandTypeReg, 1 , (_kX86RegTypePatchedGpbHi << 8) | 6, kInvalidValue, 0, 0 },
  { kOperandTypeReg, 1 , (_kX86RegTypePatchedGpbHi << 8) | 7, kInvalidValue, 0, 0 }
};

template<int Arch>
static Error ASMJIT_CDECL X86Assembler_emit(Assembler* self_, uint32_t code, const Operand* o0, const Operand* o1, const Operand* o2, const Operand* o3) {
  X86Assembler* self = static_cast<X86Assembler*>(self_);

  uint8_t* cursor = self->getCursor();
  uint32_t encoded = o0->getOp() + (o1->getOp() << 3) + (o2->getOp() << 6);
  uint32_t options = self->getInstOptionsAndReset();

  // Invalid instruction.
  if (code >= _kX86InstIdCount) {
    self->_comment = NULL;
    return self->setError(kErrorUnknownInst);
  }

  // Instruction opcode.
  uint32_t opCode;
  // MODR/R opcode or register code.
  uint32_t opReg;

  // REX or VEX prefix data.
  //
  // REX:
  //   0x0008 - REX.W.
  //   0x0040 - Always emit REX prefix.
  //
  // AVX:
  //   0x0008 - AVX.W.
  //   0xF000 - VVVV, zeros by default, see `kVexVVVV`.
  //
  uint32_t opX;

  // MOD/RM, both rmReg and rmMem should refer to the same variable since they
  // are never used together - either rmReg or rmMem.
  union {
    // MODR/M - register code.
    uintptr_t rmReg;
    // MODR/M - Memory operand.
    const X86Mem* rmMem;
  };

  // Immediate value.
  int64_t imVal;
  // Immediate length.
  uint32_t imLen = 0;

  // Memory operand base register index.
  uint32_t mBase;
  // Memory operand index register index.
  uint32_t mIndex;

  // Label.
  LabelData* label;
  // Displacement offset
  int32_t dispOffset;
  // Displacement size.
  uint32_t dispSize = 0;
  // Displacement relocation id.
  intptr_t relocId;

#if defined(ASMJIT_DEBUG)
  bool assertIllegal = false;
#endif // ASMJIT_DEBUG

  const X86InstInfo& info = _x86InstInfo[code];
  const X86InstExtendedInfo& extendedInfo = info.getExtendedInfo();

  // Grow request happens rarely. C++ compiler generates better code if it is
  // handled at the end of the function.
  if ((size_t)(self->_end - cursor) < 16)
    goto _GrowBuffer;

  // --------------------------------------------------------------------------
  // [Prepare]
  // --------------------------------------------------------------------------

_Prepare:
  opCode = info.getPrimaryOpCode();
  opReg  = opCode >> kX86InstOpCode_O_Shift;
  opX    = extendedInfo.getInstFlags() >> (15 - 3);

  if (Arch == kArchX86) {
    // AVX.W prefix.
    opX &= 0x08;

    // Check if one or more register operand is one of AH, BH, CH, or DH and
    // patch them to ensure that the binary code with correct byte-index (4-7)
    // is generated.
    if (o0->isRegType(kX86RegTypeGpbHi))
      o0 = (const Operand*)(&x86PatchedHiRegs[static_cast<const X86Reg*>(o0)->getRegIndex()]);

    if (o1->isRegType(kX86RegTypeGpbHi))
      o1 = (const Operand*)(&x86PatchedHiRegs[static_cast<const X86Reg*>(o1)->getRegIndex()]);
  }
  else {
    ASMJIT_ASSERT(kX86InstOptionRex == 0x40);

    // AVX.W prefix and REX prefix.
    opX |= options;
    opX &= 0x48;

    // Check if one or more register operand is one of BPL, SPL, SIL, DIL and
    // force a REX prefix in such case.
    if (X86Reg::isGpbReg(*o0)) {
      uint32_t index = static_cast<const X86Reg*>(o0)->getRegIndex();
      if (static_cast<const X86Reg*>(o0)->isGpbLo()) {
        opX |= (index >= 4) << kRexShift;
      }
      else {
        opX |= kRexForbidden;
        o0 = reinterpret_cast<const Operand*>(&x86PatchedHiRegs[index]);
      }
    }

    if (X86Reg::isGpbReg(*o1)) {
      uint32_t index = static_cast<const X86Reg*>(o1)->getRegIndex();
      if (static_cast<const X86Reg*>(o1)->isGpbLo()) {
        opX |= (index >= 4) << kRexShift;
      }
      else {
        opX |= kRexForbidden;
        o1 = reinterpret_cast<const Operand*>(&x86PatchedHiRegs[index]);
      }
    }
  }

  // --------------------------------------------------------------------------
  // [Lock-Prefix]
  // --------------------------------------------------------------------------

  if (options & kX86InstOptionLock) {
    if (!extendedInfo.isLockable())
      goto _IllegalInst;
    EMIT_BYTE(0xF0);
  }

  // --------------------------------------------------------------------------
  // [Group]
  // --------------------------------------------------------------------------

  switch (info.getInstGroup()) {
    // ------------------------------------------------------------------------
    // [None]
    // ------------------------------------------------------------------------

    case kX86InstGroupNone:
      goto _EmitDone;

    // ------------------------------------------------------------------------
    // [X86]
    // ------------------------------------------------------------------------

    case kX86InstGroupX86Op_66H:
      ADD_66H_P(true);
      // ... Fall through ...

    case kX86InstGroupX86Op:
      goto _EmitX86Op;

    case kX86InstGroupX86Rm_B:
      opCode += o0->getSize() != 1;
      // ... Fall through ...

    case kX86InstGroupX86Rm:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86RmReg:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86RegRm:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86M:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Arith:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode +=(o0->getSize() != 1) + 2;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode +=(o0->getSize() != 1) + 2;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The remaining instructions use 0x80 opcode.
      opCode = 0x80;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = IntUtil::isInt8(imVal) ? static_cast<uint32_t>(1) : IntUtil::iMin<uint32_t>(o0->getSize(), 4);
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Alternate Form - AL, AX, EAX, RAX.
        if (rmReg == 0 && (o0->getSize() == 1 || imLen != 1)) {
          opCode = ((opReg << 3) | (0x04 + (o0->getSize() != 1)));
          imLen = IntUtil::iMin<uint32_t>(o0->getSize(), 4);
          goto _EmitX86OpI;
        }

        opCode += o0->getSize() != 1 ? (imLen != 1 ? 1 : 3) : 0;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        uint32_t memSize = o0->getSize();

        if (memSize == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = IntUtil::isInt8(imVal) ? static_cast<uint32_t>(1) : IntUtil::iMin<uint32_t>(memSize, 4);

        opCode += memSize != 1 ? (imLen != 1 ? 1 : 3) : 0;
        ADD_66H_P_BY_SIZE(memSize);
        ADD_REX_W_BY_SIZE(memSize);

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86BSwap:
      if (encoded == ENC_OPS(Reg, None, None)) {
        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        opCode += opReg & 0x7;

        ADD_REX_W_BY_SIZE(o0->getSize());
        ADD_REX_B(opReg);
        goto _EmitX86Op;
      }
      break;

    case kX86InstGroupX86BTest:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The remaining instructions use the secondary opcode/r.
      imVal = static_cast<const Imm*>(o1)->getInt64();
      imLen = 1;

      opCode = extendedInfo.getSecondaryOpCode();
      opReg = opCode >> kX86InstOpCode_O_Shift;

      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Call:
      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        goto _EmitJmpOrCallAbs;
      }

      if (encoded == ENC_OPS(Label, None, None)) {
        label = self->getLabelData(static_cast<const Label*>(o0)->getId());
        if (label->offset != -1) {
          // Bound label.
          static const intptr_t kRel32Size = 5;
          intptr_t offs = label->offset - (intptr_t)(cursor - self->_buffer);

          ASMJIT_ASSERT(offs <= 0);
          EMIT_OP(opCode);
          EMIT_DWORD(static_cast<int32_t>(offs - kRel32Size));
        }
        else {
          // Non-bound label.
          EMIT_OP(opCode);
          dispOffset = -4;
          dispSize = 4;
          relocId = -1;
          goto _EmitDisplacement;
        }
        goto _EmitDone;
      }
      break;

    case kX86InstGroupX86Enter:
      if (encoded == ENC_OPS(Imm, Imm, None)) {
        EMIT_BYTE(0xC8);
        EMIT_WORD(static_cast<const Imm*>(o1)->getUInt16());
        EMIT_BYTE(static_cast<const Imm*>(o0)->getUInt8());
        goto _EmitDone;
      }
      break;

    case kX86InstGroupX86Imul:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        opCode = 0xF6 + (o0->getSize() != 1);

        opReg = 5;
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        opCode = 0xF6 + (o0->getSize() != 1);

        opReg = 5;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The following instructions use 0x0FAF opcode.
      opCode &= kX86InstOpCode_PP_66;
      opCode |= kX86InstOpCode_MM_0F | 0xAF;

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();

        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);

        goto _EmitX86M;
      }

      // The following instructions use 0x69/0x6B opcode.
      opCode &= kX86InstOpCode_PP_66;
      opCode |= 0x6B;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        if (!IntUtil::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = opReg;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        if (!IntUtil::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        if (!IntUtil::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86IncDec:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();

        // INC r16|r32 is not encodable in 64-bit mode.
        if (Arch == kArchX86 && (o0->getSize() == 2 || o0->getSize() == 4)) {
          opCode &= kX86InstOpCode_PP_66;
          opCode |= extendedInfo.getSecondaryOpCode() + (static_cast<uint32_t>(rmReg) & 0x7);
          goto _EmitX86Op;
        }
        else {
          opCode += o0->getSize() != 1;
          goto _EmitX86R;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        opCode += o0->getSize() != 1;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Int:
      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        uint8_t imm8 = static_cast<uint8_t>(imVal & 0xFF);

        if (imm8 == 0x03) {
          EMIT_OP(opCode);
        }
        else {
          EMIT_OP(opCode + 1);
          EMIT_BYTE(imm8);
        }
        goto _EmitDone;
      }
      break;

    case kX86InstGroupX86Jcc:
      if (encoded == ENC_OPS(Label, None, None)) {
        label = self->getLabelData(static_cast<const Label*>(o0)->getId());

        if (self->hasFeature(kCodeGenPredictedJumps)) {
          if (options & kInstOptionTaken)
            EMIT_BYTE(0x3E);
          if (options & kInstOptionNotTaken)
            EMIT_BYTE(0x2E);
        }

        if (label->offset != -1) {
          // Bound label.
          static const intptr_t kRel8Size = 2;
          static const intptr_t kRel32Size = 6;

          intptr_t offs = label->offset - (intptr_t)(cursor - self->_buffer);
          ASMJIT_ASSERT(offs <= 0);

          if ((options & kInstOptionLongForm) == 0 && IntUtil::isInt8(offs - kRel8Size)) {
            EMIT_OP(opCode);
            EMIT_BYTE(offs - kRel8Size);

            options |= kInstOptionShortForm;
            goto _EmitDone;
          }
          else {
            EMIT_BYTE(0x0F);
            EMIT_OP(opCode + 0x10);
            EMIT_DWORD(static_cast<int32_t>(offs - kRel32Size));

            options &= ~kInstOptionShortForm;
            goto _EmitDone;
          }
        }
        else {
          // Non-bound label.
          if (options & kInstOptionShortForm) {
            EMIT_OP(opCode);
            dispOffset = -1;
            dispSize = 1;
            relocId = -1;
            goto _EmitDisplacement;
          }
          else {
            EMIT_BYTE(0x0F);
            EMIT_OP(opCode + 0x10);
            dispOffset = -4;
            dispSize = 4;
            relocId = -1;
            goto _EmitDisplacement;
          }
        }
      }
      break;

    case kX86InstGroupX86Jecxz:
      if (encoded == ENC_OPS(Reg, Label, None)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->getRegIndex() == kX86RegIndexCx);

        if ((Arch == kArchX86 && o0->getSize() == 2) ||
            (Arch == kArchX64 && o0->getSize() == 4)) {
          EMIT_BYTE(0x67);
        }

        EMIT_BYTE(0xE3);
        label = self->getLabelData(static_cast<const Label*>(o1)->getId());

        if (label->offset != -1) {
          // Bound label.
          intptr_t offs = label->offset - (intptr_t)(cursor - self->_buffer) - 1;
          if (!IntUtil::isInt8(offs))
            goto _IllegalInst;

          EMIT_BYTE(offs);
          goto _EmitDone;
        }
        else {
          // Non-bound label.
          dispOffset = -1;
          dispSize = 1;
          relocId = -1;
          goto _EmitDisplacement;
        }
      }
      break;

    case kX86InstGroupX86Jmp:
      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode (0xE9).
      opCode = 0xE9;

      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        goto _EmitJmpOrCallAbs;
      }

      if (encoded == ENC_OPS(Label, None, None)) {
        label = self->getLabelData(static_cast<const Label*>(o0)->getId());
        if (label->offset != -1) {
          // Bound label.
          const intptr_t kRel8Size = 2;
          const intptr_t kRel32Size = 5;

          intptr_t offs = label->offset - (intptr_t)(cursor - self->_buffer);

          if ((options & kInstOptionLongForm) == 0 && IntUtil::isInt8(offs - kRel8Size)) {
            options |= kInstOptionShortForm;

            EMIT_BYTE(0xEB);
            EMIT_BYTE(offs - kRel8Size);
            goto _EmitDone;
          }
          else {
            options &= ~kInstOptionShortForm;

            EMIT_BYTE(0xE9);
            EMIT_DWORD(static_cast<int32_t>(offs - kRel32Size));
            goto _EmitDone;
          }
        }
        else {
          // Non-bound label.
          if ((options & kInstOptionShortForm) != 0) {
            EMIT_BYTE(0xEB);
            dispOffset = -1;
            dispSize = 1;
            relocId = -1;
            goto _EmitDisplacement;
          }
          else {
            EMIT_BYTE(0xE9);
            dispOffset = -4;
            dispSize = 4;
            relocId = -1;
            goto _EmitDisplacement;
          }
        }
      }
      break;

    case kX86InstGroupX86Lea:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Mov:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();

        // Sreg <- Reg
        if (static_cast<const X86Reg*>(o0)->isSeg()) {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isGpw() ||
                        static_cast<const X86Reg*>(o1)->isGpd() ||
                        static_cast<const X86Reg*>(o1)->isGpq() );
          opCode = 0x8E;
          ADD_66H_P_BY_SIZE(o1->getSize());
          ADD_REX_W_BY_SIZE(o1->getSize());
          goto _EmitX86R;
        }

        // Reg <- Sreg
        if (static_cast<const X86Reg*>(o1)->isSeg()) {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->isGpw() ||
                        static_cast<const X86Reg*>(o0)->isGpd() ||
                        static_cast<const X86Reg*>(o0)->isGpq() );
          opCode = 0x8C;
          ADD_66H_P_BY_SIZE(o0->getSize());
          ADD_REX_W_BY_SIZE(o0->getSize());
          goto _EmitX86R;
        }
        // Reg <- Reg
        else {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->isGpb() ||
                        static_cast<const X86Reg*>(o0)->isGpw() ||
                        static_cast<const X86Reg*>(o0)->isGpd() ||
                        static_cast<const X86Reg*>(o0)->isGpq() );
          opCode = 0x8A + (o0->getSize() != 1);
          ADD_66H_P_BY_SIZE(o0->getSize());
          ADD_REX_W_BY_SIZE(o0->getSize());
          goto _EmitX86R;
        }
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);

        // Sreg <- Mem
        if (static_cast<const X86Reg*>(o0)->isRegType(kX86RegTypeSeg)) {
          opCode = 0x8E;
          opReg--;
          ADD_66H_P_BY_SIZE(o1->getSize());
          ADD_REX_W_BY_SIZE(o1->getSize());
          goto _EmitX86M;
        }
        // Reg <- Mem
        else {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->isGpb() ||
                        static_cast<const X86Reg*>(o0)->isGpw() ||
                        static_cast<const X86Reg*>(o0)->isGpd() ||
                        static_cast<const X86Reg*>(o0)->isGpq() );
          opCode = 0x8A + (o0->getSize() != 1);
          ADD_66H_P_BY_SIZE(o0->getSize());
          ADD_REX_W_BY_SIZE(o0->getSize());
          goto _EmitX86M;
        }
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);

        // X86Mem <- Sreg
        if (static_cast<const X86Reg*>(o1)->isSeg()) {
          opCode = 0x8C;
          ADD_66H_P_BY_SIZE(o0->getSize());
          ADD_REX_W_BY_SIZE(o0->getSize());
          goto _EmitX86M;
        }
        // X86Mem <- Reg
        else {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isGpb() ||
                        static_cast<const X86Reg*>(o1)->isGpw() ||
                        static_cast<const X86Reg*>(o1)->isGpd() ||
                        static_cast<const X86Reg*>(o1)->isGpq() );
          opCode = 0x88 + (o1->getSize() != 1);
          ADD_66H_P_BY_SIZE(o1->getSize());
          ADD_REX_W_BY_SIZE(o1->getSize());
          goto _EmitX86M;
        }
      }

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        // 64-bit immediate in 64-bit mode is allowed.
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = o0->getSize();

        opReg = 0;
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();

        // Optimize instruction size by using 32-bit immediate if possible.
        if (Arch == kArchX64 && imLen == 8 && IntUtil::isInt32(imVal)) {
          opCode = 0xC7;
          ADD_REX_W(1);
          imLen = 4;
          goto _EmitX86R;
        }
        else {
          opCode = 0xB0 + (static_cast<uint32_t>(o0->getSize() != 1) << 3) + (static_cast<uint32_t>(rmReg) & 0x7);
          ADD_REX_W_BY_SIZE(imLen);
          ADD_REX_B(rmReg);
          goto _EmitX86OpI;
        }
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        uint32_t memSize = o0->getSize();

        if (memSize == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = IntUtil::iMin<uint32_t>(memSize, 4);

        opCode = 0xC6 + (memSize != 1);
        opReg = 0;
        ADD_66H_P_BY_SIZE(memSize);
        ADD_REX_W_BY_SIZE(memSize);

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86MovSxZx:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86MovSxd:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_REX_W(true);

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_REX_W(true);

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86MovPtr:
      if (encoded == ENC_OPS(Reg, Imm, None)) {
        ASMJIT_ASSERT(static_cast<const X86GpReg*>(o0)->getRegIndex() == 0);

        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = self->_regSize;
        goto _EmitX86OpI;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Imm, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const X86GpReg*>(o1)->getRegIndex() == 0);

        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        imVal = static_cast<const Imm*>(o0)->getInt64();
        imLen = self->_regSize;
        goto _EmitX86OpI;
      }
      break;

    case kX86InstGroupX86Push:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (o0->isRegType(kX86RegTypeSeg)) {
          uint32_t segment = static_cast<const X86SegReg*>(o0)->getRegIndex();
          ASMJIT_ASSERT(segment < kX86SegCount);

          if (segment >= kX86SegFs)
            EMIT_BYTE(0x0F);

          EMIT_BYTE(x86OpCodePushSeg[segment]);
          goto _EmitDone;
        }
        else {
          goto _GroupPop_Gp;
        }
      }

      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        imLen = IntUtil::isInt8(imVal) ? 1 : 4;

        EMIT_BYTE(imLen == 1 ? 0x6A : 0x68);
        goto _EmitImm;
      }
      // ... Fall through ...

    case kX86InstGroupX86Pop:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (o0->isRegType(kX86RegTypeSeg)) {
          uint32_t segment = static_cast<const X86SegReg*>(o0)->getRegIndex();
          ASMJIT_ASSERT(segment < kX86SegCount);

          if (segment >= kX86SegFs)
            EMIT_BYTE(0x0F);

          EMIT_BYTE(x86OpCodePopSeg[segment]);
          goto _EmitDone;
        }
        else {
_GroupPop_Gp:
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->getSize() == 2 ||
                        static_cast<const X86Reg*>(o0)->getSize() == self->_regSize);

          opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
          opCode = extendedInfo.getSecondaryOpCode() + (opReg & 7);

          ADD_66H_P_BY_SIZE(o0->getSize());
          ADD_REX_B(opReg);

          goto _EmitX86Op;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        ADD_66H_P_BY_SIZE(o0->getSize());

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Rep:
      // Emit REP 0xF2 or 0xF3 prefix first.
      EMIT_BYTE(0xF2 + opReg);
      goto _EmitX86Op;

    case kX86InstGroupX86Ret:
      if (encoded == ENC_OPS(None, None, None)) {
        EMIT_BYTE(0xC3);
        goto _EmitDone;
      }

      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        if (imVal == 0) {
          EMIT_BYTE(0xC3);
          goto _EmitDone;
        }
        else {
          EMIT_BYTE(0xC2);
          imLen = 2;
          goto _EmitImm;
        }
      }
      break;

    case kX86InstGroupX86Rot:
      opCode += o0->getSize() != 1;
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        opCode += 2;
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        opCode += 2;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64() & 0xFF;
        imLen = imVal != 1;
        if (imLen)
          opCode -= 0x10;
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64() & 0xFF;
        imLen = imVal != 1;
        if (imLen)
          opCode -= 0x10;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Set:
      if (encoded == ENC_OPS(Reg, None, None)) {
        ASMJIT_ASSERT(o0->getSize() == 1);

        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        ASMJIT_ASSERT(o0->getSize() <= 1);

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Shlrd:
      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The following instructions use opCode + 1.
      opCode++;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o2)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Reg)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o2)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));

        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Test:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode() + (o0->getSize() != 1);
      opReg = opCode >> kX86InstOpCode_O_Shift;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = IntUtil::iMin<uint32_t>(o0->getSize(), 4);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Alternate Form - AL, AX, EAX, RAX.
        if (static_cast<const X86GpReg*>(o0)->getRegIndex() == 0) {
          opCode &= kX86InstOpCode_PP_66;
          opCode |= 0xA8 + (o0->getSize() != 1);
          goto _EmitX86OpI;
        }

        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = IntUtil::iMin<uint32_t>(o0->getSize(), 4);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupX86Xchg:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      // ... fall through ...

    case kX86InstGroupX86Xadd:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o0)->getRegIndex();

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Special opcode for 'xchg ?ax, reg'.
        if (code == kX86InstIdXchg && o0->getSize() > 1 && (opReg == 0 || rmReg == 0)) {
          // One of them is zero, it doesn't matter if the instruction's form is
          // 'xchg ?ax, reg' or 'xchg reg, ?ax'.
          opReg += rmReg;

          // Rex.B (0x01).
          if (Arch == kArchX64) {
            opX += opReg >> 3;
            opReg &= 0x7;
          }

          opCode &= kX86InstOpCode_PP_66;
          opCode |= 0x90 + opReg;
          goto _EmitX86Op;
        }

        opCode += o0->getSize() != 1;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Fpu]
    // ------------------------------------------------------------------------

    case kX86InstGroupFpuOp:
      goto _EmitFpuOp;

    case kX86InstGroupFpuArith:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86FpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86FpReg*>(o1)->getRegIndex();
        rmReg += opReg;

        // We switch to the alternative opcode if the first operand is zero.
        if (opReg == 0) {
_EmitFpArith_Reg:
          opCode = 0xD800 + ((opCode >> 8) & 0xFF) + static_cast<uint32_t>(rmReg);
          goto _EmitFpuOp;
        }
        else {
          opCode = 0xDC00 + ((opCode >> 0) & 0xFF) + static_cast<uint32_t>(rmReg);
          goto _EmitFpuOp;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        // 0xD8/0xDC, depends on the size of the memory operand; opReg has been
        // set already.
_EmitFpArith_Mem:
        opCode = (o0->getSize() == 4) ? 0xD8 : 0xDC;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupFpuCom:
      if (encoded == ENC_OPS(None, None, None)) {
        rmReg = 1;
        goto _EmitFpArith_Reg;
      }

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = static_cast<const X86FpReg*>(o0)->getRegIndex();
        goto _EmitFpArith_Reg;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        goto _EmitFpArith_Mem;
      }
      break;

    case kX86InstGroupFpuFldFst:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);

        if (o0->getSize() == 4 && info.hasInstFlag(kX86InstFlagMem4)) {
          goto _EmitX86M;
        }

        if (o0->getSize() == 8 && info.hasInstFlag(kX86InstFlagMem8)) {
          opCode += 4;
          goto _EmitX86M;
        }

        if (o0->getSize() == 10 && info.hasInstFlag(kX86InstFlagMem10)) {
          opCode = extendedInfo.getSecondaryOpCode();
          opReg = opCode >> kX86InstOpCode_O_Shift;
          goto _EmitX86M;
        }
      }

      if (encoded == ENC_OPS(Reg, None, None)) {
        if (code == kX86InstIdFld) {
          opCode = 0xD9C0 + static_cast<const X86FpReg*>(o0)->getRegIndex();
          goto _EmitFpuOp;
        }

        if (code == kX86InstIdFst) {
          opCode = 0xDDD0 + static_cast<const X86FpReg*>(o0)->getRegIndex();
          goto _EmitFpuOp;
        }

        if (code == kX86InstIdFstp) {
          opCode = 0xDDD8 + static_cast<const X86FpReg*>(o0)->getRegIndex();
          goto _EmitFpuOp;
        }
      }
      break;


    case kX86InstGroupFpuM:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);

        if (o0->getSize() == 2 && info.hasInstFlag(kX86InstFlagMem2)) {
          opCode += 4;
          goto _EmitX86M;
        }

        if (o0->getSize() == 4 && info.hasInstFlag(kX86InstFlagMem4)) {
          goto _EmitX86M;
        }

        if (o0->getSize() == 8 && info.hasInstFlag(kX86InstFlagMem8)) {
          opCode = extendedInfo.getSecondaryOpCode();
          opReg = opCode >> kX86InstOpCode_O_Shift;
          goto _EmitX86M;
        }
      }
      break;

    case kX86InstGroupFpuRDef:
      if (encoded == ENC_OPS(None, None, None)) {
        opCode += 1;
        goto _EmitFpuOp;
      }
      // ... Fall through ...

    case kX86InstGroupFpuR:
      if (encoded == ENC_OPS(Reg, None, None)) {
        opCode += static_cast<const X86FpReg*>(o0)->getRegIndex();
        goto _EmitFpuOp;
      }
      break;

    case kX86InstGroupFpuStsw:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (static_cast<const X86GpReg*>(o0)->getRegIndex() != 0)
          goto _IllegalInst;

        opCode = extendedInfo.getSecondaryOpCode();
        goto _EmitX86Op;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Ext]
    // ------------------------------------------------------------------------

    case kX86InstGroupExtCrc:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const Reg*>(o0)->getRegType() == kX86RegTypeGpd ||
                      static_cast<const Reg*>(o0)->getRegType() == kX86RegTypeGpq);

        opCode += o0->getSize() != 1;
        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ASMJIT_ASSERT(static_cast<const Reg*>(o0)->getRegType() == kX86RegTypeGpd ||
                      static_cast<const Reg*>(o0)->getRegType() == kX86RegTypeGpq);

        opCode += o0->getSize() != 1;
        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtExtract:
      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ADD_66H_P(static_cast<const X86Reg*>(o1)->isXmm());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        // Secondary opcode for 'pextrw' instruction (SSE2).
        opCode = extendedInfo.getSecondaryOpCode();
        ADD_66H_P(static_cast<const X86Reg*>(o1)->isXmm());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtFence:
      if (Arch == kArchX64 && opX) {
        EMIT_BYTE(0x40 | opX);
      }

      EMIT_BYTE(0x0F);
      EMIT_OP(opCode);
      EMIT_BYTE(0xC0 | (opReg << 3));
      goto _EmitDone;

    case kX86InstGroupExtMov:
    case kX86InstGroupExtMovNoRexW:
      ASMJIT_ASSERT(extendedInfo._opFlags[0] != 0);
      ASMJIT_ASSERT(extendedInfo._opFlags[1] != 0);

      // Check parameters Gpd|Gpq|Mm|Xmm <- Gpd|Gpq|Mm|Xmm|X86Mem|Imm.
      ASMJIT_ASSERT(!((o0->isMem()                   && (extendedInfo._opFlags[0] & kX86InstOpMem) == 0) ||
                      (o0->isRegType(kX86RegTypeMm ) && (extendedInfo._opFlags[0] & kX86InstOpMm ) == 0) ||
                      (o0->isRegType(kX86RegTypeXmm) && (extendedInfo._opFlags[0] & kX86InstOpXmm) == 0) ||
                      (o0->isRegType(kX86RegTypeGpd) && (extendedInfo._opFlags[0] & kX86InstOpGd ) == 0) ||
                      (o0->isRegType(kX86RegTypeGpq) && (extendedInfo._opFlags[0] & kX86InstOpGq ) == 0) ||
                      (o1->isMem()                   && (extendedInfo._opFlags[1] & kX86InstOpMem) == 0) ||
                      (o1->isRegType(kX86RegTypeMm ) && (extendedInfo._opFlags[1] & kX86InstOpMm ) == 0) ||
                      (o1->isRegType(kX86RegTypeXmm) && (extendedInfo._opFlags[1] & kX86InstOpXmm) == 0) ||
                      (o1->isRegType(kX86RegTypeGpd) && (extendedInfo._opFlags[1] & kX86InstOpGd ) == 0) ||
                      (o1->isRegType(kX86RegTypeGpq) && (extendedInfo._opFlags[1] & kX86InstOpGq ) == 0) ));

      // Gp|Mm|Xmm <- Gp|Mm|Xmm
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_REX_W(static_cast<const X86Reg*>(o0)->isGpq() && (info.getInstGroup() != kX86InstGroupExtMovNoRexW));
        ADD_REX_W(static_cast<const X86Reg*>(o1)->isGpq() && (info.getInstGroup() != kX86InstGroupExtMovNoRexW));

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      // Gp|Mm|Xmm <- Mem
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_REX_W(static_cast<const X86Reg*>(o0)->isGpq() && (info.getInstGroup() != kX86InstGroupExtMovNoRexW));

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      // The following instruction uses opCode[1].
      opCode = extendedInfo.getSecondaryOpCode();

      // X86Mem <- Gp|Mm|Xmm
      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ADD_REX_W(static_cast<const X86Reg*>(o1)->isGpq() && (info.getInstGroup() != kX86InstGroupExtMovNoRexW));

        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtMovBe:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = static_cast<const X86GpReg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = static_cast<const X86GpReg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtMovD:
_EmitMmMovD:
      opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
      ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm());

      // Mm/Xmm <- Gp
      if (encoded == ENC_OPS(Reg, Reg, None) && static_cast<const X86Reg*>(o1)->isGp()) {
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      // Mm/Xmm <- Mem
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
      ADD_66H_P(static_cast<const X86Reg*>(o1)->isXmm());

      // Gp <- Mm/Xmm
      if (encoded == ENC_OPS(Reg, Reg, None) && static_cast<const X86Reg*>(o0)->isGp()) {
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }

      // X86Mem <- Mm/Xmm
      if (encoded == ENC_OPS(Mem, Reg, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtMovQ:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();

        // Mm <- Mm
        if (static_cast<const X86Reg*>(o0)->isMm() && static_cast<const X86Reg*>(o1)->isMm()) {
          opCode = kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F | 0x6F;
          goto _EmitX86R;
        }

        // Xmm <- Xmm
        if (static_cast<const X86Reg*>(o0)->isXmm() && static_cast<const X86Reg*>(o1)->isXmm()) {
          opCode = kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F | 0x7E;
          goto _EmitX86R;
        }

        // Mm <- Xmm (Movdq2q)
        if (static_cast<const X86Reg*>(o0)->isMm() && static_cast<const X86Reg*>(o1)->isXmm()) {
          opCode = kX86InstOpCode_PP_F2 | kX86InstOpCode_MM_0F | 0xD6;
          goto _EmitX86R;
        }

        // Xmm <- Mm (Movq2dq)
        if (static_cast<const X86Reg*>(o0)->isXmm() && static_cast<const X86Reg*>(o1)->isMm()) {
          opCode = kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F | 0xD6;
          goto _EmitX86R;
        }
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);

        // Mm <- Mem
        if (static_cast<const X86Reg*>(o0)->isMm()) {
          opCode = kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F | 0x6F;
          goto _EmitX86M;
        }

        // Xmm <- Mem
        if (static_cast<const X86Reg*>(o0)->isXmm()) {
          opCode = kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F | 0x7E;
          goto _EmitX86M;
        }
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);

        // X86Mem <- Mm
        if (static_cast<const X86Reg*>(o1)->isMm()) {
          opCode = kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F | 0x7F;
          goto _EmitX86M;
        }

        // X86Mem <- Xmm
        if (static_cast<const X86Reg*>(o1)->isXmm()) {
          opCode = kX86InstOpCode_PP_66 | kX86InstOpCode_MM_0F | 0xD6;
          goto _EmitX86M;
        }
      }

      if (Arch == kArchX64) {
        // Movq in other case is simply a promoted MOVD instruction to 64-bit.
        ADD_REX_W(true);

        opCode = kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F | 0x6E;
        goto _EmitMmMovD;
      }
      break;

    case kX86InstGroupExtPrefetch:
      if (encoded == ENC_OPS(Mem, Imm, None)) {
        opReg = static_cast<const Imm*>(o1)->getUInt32() & 0x3;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtRm_PQ:
      ADD_66H_P(o0->isRegType(kX86RegTypeXmm) || o1->isRegType(kX86RegTypeXmm));
      // ... Fall through ...

    case kX86InstGroupExtRm_Q:
      ADD_REX_W(o0->isRegType(kX86RegTypeGpq) || o1->isRegType(kX86RegTypeGpq) || (o1->isMem() && o1->getSize() == 8));
      // ... Fall through ...

    case kX86InstGroupExtRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtRm_P:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm() | static_cast<const X86Reg*>(o1)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtRmRi:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg = opCode >> kX86InstOpCode_O_Shift;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }
      break;

    case kX86InstGroupExtRmRi_P:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm() | static_cast<const X86Reg*>(o1)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg = opCode >> kX86InstOpCode_O_Shift;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm());

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitX86R;
      }
      break;

    case kX86InstGroupExtRmi:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstGroupExtRmi_P:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm() | static_cast<const X86Reg*>(o1)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        ADD_66H_P(static_cast<const X86Reg*>(o0)->isXmm());

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Group - 3dNow]
    // ------------------------------------------------------------------------

    case kX86InstGroup3dNow:
      // Every 3dNow instruction starts with 0x0F0F and the actual opcode is
      // stored as 8-bit immediate.
      imVal = opCode & 0xFF;
      imLen = 1;

      opCode = kX86InstOpCode_MM_0F | 0x0F;
      opReg = static_cast<const X86Reg*>(o0)->getRegIndex();

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Avx]
    // ------------------------------------------------------------------------

    case kX86InstGroupAvxOp:
      goto _EmitAvxOp;

    case kX86InstGroupAvxM:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxMr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxMr:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxMri_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxMri:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRm_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRmi:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvm_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvm:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
_EmitAvxRvm:
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvmr:
      if (!o3->isReg())
        goto _IllegalInst;

      imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvmi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRmv:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRmvi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRmMr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRmMr:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmRmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvmRmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }

      // The following instructions use the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmMr:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmMvr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvmMvr:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvmVmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvmVmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();
      opReg = opCode >> kX86InstOpCode_O_Shift;

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxVm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxVmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxVmi:
      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxRvrmRvmr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupAvxRvrmRvmr:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();

        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = static_cast<const X86Reg*>(o2)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o3);

        ADD_VEX_W(true);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);

        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxMovSsSd:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        goto _EmitAvxRvm;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opX  |= static_cast<const X86Reg*>(o0)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstGroupAvxGatherEx:
      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);

        uint32_t vSib = rmMem->getVSib();
        if (vSib == kX86MemVSibGpz)
          goto _IllegalInst;

        ADD_VEX_L(vSib == kX86MemVSibYmm);
        goto _EmitAvxV;
      }
      break;

    case kX86InstGroupAvxGather:
      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);

        uint32_t vSib = rmMem->getVSib();
        if (vSib == kX86MemVSibGpz)
          goto _IllegalInst;

        ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o2)->isYmm());
        goto _EmitAvxV;
      }
      break;

    // ------------------------------------------------------------------------
    // [FMA4]
    // ------------------------------------------------------------------------

    case kX86InstGroupFma4_P:
      // It's fine to just check the first operand, second is just for sanity.
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupFma4:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();

        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = static_cast<const X86Reg*>(o2)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o3);

        ADD_VEX_W(true);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);

        goto _EmitAvxM;
      }
      break;

    // ------------------------------------------------------------------------
    // [XOP]
    // ------------------------------------------------------------------------

    case kX86InstGroupXopRm_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupXopRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitXopM;
      }
      break;

    case kX86InstGroupXopRvmRmv:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();

        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);

        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      break;

    case kX86InstGroupXopRvmRmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o2)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o1);

        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o1)->getRegIndex();
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmMem = static_cast<const X86Mem*>(o1);
        goto _EmitXopM;
      }
      break;

    case kX86InstGroupXopRvmr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupXopRvmr:
      if (!o3->isReg())
        goto _IllegalInst;

      imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitXopM;
      }
      break;

    case kX86InstGroupXopRvmi_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupXopRvmi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);
        goto _EmitXopM;
      }
      break;

    case kX86InstGroupXopRvrmRvmr_P:
      ADD_VEX_L(static_cast<const X86Reg*>(o0)->isYmm() | static_cast<const X86Reg*>(o1)->isYmm());
      // ... Fall through ...

    case kX86InstGroupXopRvrmRvmr:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmReg = static_cast<const X86Reg*>(o2)->getRegIndex();

        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = static_cast<const X86Reg*>(o2)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o3);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = static_cast<const X86Reg*>(o3)->getRegIndex() << 4;
        imLen = 1;

        opReg = static_cast<const X86Reg*>(o0)->getRegIndex();
        opX  |= static_cast<const X86Reg*>(o1)->getRegIndex() << kVexVVVVShift;
        rmMem = static_cast<const X86Mem*>(o2);

        goto _EmitXopM;
      }
      break;
  }

  // --------------------------------------------------------------------------
  // [Illegal]
  // --------------------------------------------------------------------------

_IllegalInst:
  self->setError(kErrorIllegalInst);
#if defined(ASMJIT_DEBUG)
  assertIllegal = true;
#endif // ASMJIT_DEBUG
  goto _EmitDone;

_IllegalAddr:
  self->setError(kErrorIllegalAddresing);
#if defined(ASMJIT_DEBUG)
  assertIllegal = true;
#endif // ASMJIT_DEBUG
  goto _EmitDone;

_IllegalDisp:
  self->setError(kErrorIllegalDisplacement);
#if defined(ASMJIT_DEBUG)
  assertIllegal = true;
#endif // ASMJIT_DEBUG
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - X86]
  // --------------------------------------------------------------------------

_EmitX86Op:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64 && opX) {
    opX |= 0x40;
    EMIT_BYTE(opX);
    if (opX >= kRexForbidden)
      goto _IllegalInst;
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_OP(opCode);
  goto _EmitDone;

_EmitX86OpI:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64 && opX) {
    opX |= 0x40;
    EMIT_BYTE(opX);
    if (opX >= kRexForbidden)
      goto _IllegalInst;
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_OP(opCode);
  goto _EmitImm;

_EmitX86R:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64) {
    opX += static_cast<uint32_t>(opReg & 0x08) >> 1; // Rex.R (0x04).
    opX += static_cast<uint32_t>(rmReg) >> 3;        // Rex.B (0x01).

    if (opX) {
      opX |= 0x40;
      EMIT_BYTE(opX);

      if (opX >= kRexForbidden)
        goto _IllegalInst;

      opReg &= 0x7;
      rmReg &= 0x7;
    }
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_OP(opCode);

  // ModR.
  EMIT_BYTE(x86EncodeMod(3, opReg, static_cast<uint32_t>(rmReg)));

  if (imLen != 0)
    goto _EmitImm;
  else
    goto _EmitDone;

_EmitX86M:
  ASMJIT_ASSERT(rmMem != NULL);
  ASMJIT_ASSERT(rmMem->getOp() == kOperandTypeMem);

  mBase = rmMem->getBase();
  mIndex = rmMem->getIndex();

  // Size override prefix.
  if (rmMem->hasBaseOrIndex() && rmMem->getMemType() != kMemTypeLabel) {
    if (Arch == kArchX86) {
      if (!rmMem->hasGpdBase())
        EMIT_BYTE(0x67);
    }
    else {
      if (rmMem->hasGpdBase())
        EMIT_BYTE(0x67);
    }
  }

  // Segment override prefix.
  if (rmMem->hasSegment()) {
    EMIT_BYTE(x86SegmentPrefix[rmMem->getSegment()]);
  }

  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64) {
    opX += static_cast<uint32_t>(opReg      & 8) >> 1; // Rex.R (0x04).
    opX += static_cast<uint32_t>(mIndex - 8 < 8) << 1; // Rex.X (0x02).
    opX += static_cast<uint32_t>(mBase  - 8 < 8);      // Rex.B (0x01).

    if (opX) {
      opX |= 0x40;
      EMIT_BYTE(opX);

      if (opX >= kRexForbidden)
        goto _IllegalInst;

      opReg &= 0x7;
    }

    mBase &= 0x7;
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_OP(opCode);
  // ... Fall through ...

  // --------------------------------------------------------------------------
  // [Emit - SIB]
  // --------------------------------------------------------------------------

_EmitSib:
  dispOffset = rmMem->getDisplacement();
  if (rmMem->isBaseIndexType()) {
    if (mIndex >= kInvalidReg) {
      if (mBase == kX86RegIndexSp) {
        if (dispOffset == 0) {
          // [Esp/Rsp/R12].
          EMIT_BYTE(x86EncodeMod(0, opReg, 4));
          EMIT_BYTE(x86EncodeSib(0, 4, 4));
        }
        else if (IntUtil::isInt8(dispOffset)) {
          // [Esp/Rsp/R12 + Disp8].
          EMIT_BYTE(x86EncodeMod(1, opReg, 4));
          EMIT_BYTE(x86EncodeSib(0, 4, 4));
          EMIT_BYTE(static_cast<int8_t>(dispOffset));
        }
        else {
          // [Esp/Rsp/R12 + Disp32].
          EMIT_BYTE(x86EncodeMod(2, opReg, 4));
          EMIT_BYTE(x86EncodeSib(0, 4, 4));
          EMIT_DWORD(static_cast<int32_t>(dispOffset));
        }
      }
      else if (mBase != kX86RegIndexBp && dispOffset == 0) {
        // [Base].
        EMIT_BYTE(x86EncodeMod(0, opReg, mBase));
      }
      else if (IntUtil::isInt8(dispOffset)) {
        // [Base + Disp8].
        EMIT_BYTE(x86EncodeMod(1, opReg, mBase));
        EMIT_BYTE(static_cast<int8_t>(dispOffset));
      }
      else {
        // [Base + Disp32].
        EMIT_BYTE(x86EncodeMod(2, opReg, mBase));
        EMIT_DWORD(static_cast<int32_t>(dispOffset));
      }
    }
    else {
      uint32_t shift = rmMem->getShift();

      // Esp/Rsp/R12 register can't be used as an index.
      mIndex &= 0x7;
      ASMJIT_ASSERT(mIndex != kX86RegIndexSp);

      if (mBase != kX86RegIndexBp && dispOffset == 0) {
        // [Base + Index * Scale].
        EMIT_BYTE(x86EncodeMod(0, opReg, 4));
        EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
      }
      else if (IntUtil::isInt8(dispOffset)) {
        // [Base + Index * Scale + Disp8].
        EMIT_BYTE(x86EncodeMod(1, opReg, 4));
        EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
        EMIT_BYTE(static_cast<int8_t>(dispOffset));
      }
      else {
        // [Base + Index * Scale + Disp32].
        EMIT_BYTE(x86EncodeMod(2, opReg, 4));
        EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
        EMIT_DWORD(static_cast<int32_t>(dispOffset));
      }
    }
  }
  else if (Arch == kArchX86) {
    if (mIndex >= kInvalidReg) {
      // [Disp32].
      EMIT_BYTE(x86EncodeMod(0, opReg, 5));
    }
    else {
      // [Index * Scale + Disp32].
      uint32_t shift = rmMem->getShift();
      ASMJIT_ASSERT(mIndex != kX86RegIndexSp);

      EMIT_BYTE(x86EncodeMod(0, opReg, 4));
      EMIT_BYTE(x86EncodeSib(shift, mIndex, 5));
    }

    if (rmMem->getMemType() == kMemTypeLabel) {
      // Relative->Absolute [x86 mode].
      label = self->getLabelData(rmMem->_vmem.base);
      relocId = self->_relocList.getLength();

      RelocData reloc;
      reloc.type = kRelocRelToAbs;
      reloc.size = 4;
      reloc.from = static_cast<Ptr>((uintptr_t)(cursor - self->_buffer));
      reloc.data = static_cast<SignedPtr>(dispOffset);

      if (self->_relocList.append(reloc) != kErrorOk)
        return self->setError(kErrorNoHeapMemory);

      if (label->offset != -1) {
        // Bound label.
        reloc.data += static_cast<SignedPtr>(label->offset);
        EMIT_DWORD(0);
      }
      else {
        // Non-bound label.
        dispOffset = -4 - imLen;
        dispSize = 4;
        goto _EmitDisplacement;
      }
    }
    else {
      // [Disp32].
      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
  }
  else /* if (Arch === kArchX64) */ {
    if (rmMem->getMemType() == kMemTypeLabel) {
      // [RIP + Disp32].
      label = self->getLabelData(rmMem->_vmem.base);

      // Indexing is invalid.
      if (mIndex < kInvalidReg)
        goto _IllegalDisp;

      EMIT_BYTE(x86EncodeMod(0, opReg, 5));
      dispOffset -= (4 + imLen);

      if (label->offset != -1) {
        // Bound label.
        dispOffset += label->offset - static_cast<int32_t>((intptr_t)(cursor - self->_buffer));
        EMIT_DWORD(static_cast<int32_t>(dispOffset));
      }
      else {
        // Non-bound label.
        dispSize = 4;
        relocId = -1;
        goto _EmitDisplacement;
      }
    }
    else {
      EMIT_BYTE(x86EncodeMod(0, opReg, 4));
      if (mIndex >= kInvalidReg) {
        // [Disp32].
        EMIT_BYTE(x86EncodeSib(0, 4, 5));
      }
      else {
        // [Disp32 + Index * Scale].
        mIndex &= 0x7;
        ASMJIT_ASSERT(mIndex != kX86RegIndexSp);

        uint32_t shift = rmMem->getShift();
        EMIT_BYTE(x86EncodeSib(shift, mIndex, 5));
      }

      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
  }

  if (imLen == 0)
    goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Imm]
  // --------------------------------------------------------------------------

_EmitImm:
  switch (imLen) {
    case 1: EMIT_BYTE (imVal & 0x000000FF); break;
    case 2: EMIT_WORD (imVal & 0x0000FFFF); break;
    case 4: EMIT_DWORD(imVal & 0xFFFFFFFF); break;
    case 8: EMIT_QWORD(imVal             ); break;

    default:
      ASMJIT_ASSERT(!"Reached");
  }
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Fpu]
  // --------------------------------------------------------------------------

_EmitFpuOp:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Instruction opcodes.
  EMIT_OP(opCode >> 8);
  EMIT_OP(opCode);
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Avx]
  // --------------------------------------------------------------------------

#define EMIT_AVX_M \
  ASMJIT_ASSERT(rmMem != NULL); \
  ASMJIT_ASSERT(rmMem->getOp() == kOperandTypeMem); \
  \
  if (rmMem->hasSegment()) { \
    EMIT_BYTE(x86SegmentPrefix[rmMem->getSegment()]); \
  } \
  \
  mBase = rmMem->getBase(); \
  mIndex = rmMem->getIndex(); \
  \
  { \
    uint32_t vex_XvvvvLpp; \
    uint32_t vex_rxbmmmmm; \
    \
    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift)) & 0x03; \
    vex_XvvvvLpp += (opX >> (kVexVVVVShift - 3)); \
    vex_XvvvvLpp += (opX << 4) & 0x80; \
    \
    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x1F; \
    vex_rxbmmmmm += static_cast<uint32_t>(mBase  - 8 < 8) << 5; \
    vex_rxbmmmmm += static_cast<uint32_t>(mIndex - 8 < 8) << 6; \
    \
    if (vex_rxbmmmmm != 0x01 || vex_XvvvvLpp >= 0x80 || (options & kX86InstOptionVex3) != 0) { \
      vex_rxbmmmmm |= static_cast<uint32_t>(opReg << 4) & 0x80; \
      vex_rxbmmmmm ^= 0xE0; \
      vex_XvvvvLpp ^= 0x78; \
      \
      EMIT_BYTE(kVex3Byte); \
      EMIT_BYTE(vex_rxbmmmmm); \
      EMIT_BYTE(vex_XvvvvLpp); \
      EMIT_OP(opCode); \
    } \
    else { \
      vex_XvvvvLpp |= static_cast<uint32_t>(opReg << 4) & 0x80; \
      vex_XvvvvLpp ^= 0xF8; \
      \
      EMIT_BYTE(kVex2Byte); \
      EMIT_BYTE(vex_XvvvvLpp); \
      EMIT_OP(opCode); \
    } \
  } \
  \
  mBase &= 0x7; \
  opReg &= 0x7;

_EmitAvxOp:
  {
    uint32_t vex_XvvvvLpp;

    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    vex_XvvvvLpp |= (opCode >> (kX86InstOpCode_PP_Shift));
    vex_XvvvvLpp |= 0xF8;

    // Encode 3-byte VEX prefix only if specified in options.
    if ((options & kX86InstOptionVex3) != 0) {
      uint32_t vex_rxbmmmmm = (opCode >> kX86InstOpCode_MM_Shift) | 0xE0;

      EMIT_BYTE(kVex3Byte);
      EMIT_OP(vex_rxbmmmmm);
      EMIT_OP(vex_XvvvvLpp);
      EMIT_OP(opCode);
    }
    else {
      EMIT_BYTE(kVex2Byte);
      EMIT_OP(vex_XvvvvLpp);
      EMIT_OP(opCode);
    }
  }
  goto _EmitDone;

_EmitAvxR:
  {
    uint32_t vex_XvvvvLpp;
    uint32_t vex_rxbmmmmm;

    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    vex_XvvvvLpp |= (opCode >> (kX86InstOpCode_PP_Shift));
    vex_XvvvvLpp |= (opX    >> (kVexVVVVShift - 3));
    vex_XvvvvLpp |= (opX << 4) & 0x80;

    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x1F;
    vex_rxbmmmmm |= (rmReg << 2) & 0x20;

    if (vex_rxbmmmmm != 0x01 || vex_XvvvvLpp >= 0x80 || (options & kX86InstOptionVex3) != 0) {
      vex_rxbmmmmm |= static_cast<uint32_t>(opReg & 0x08) << 4;
      vex_rxbmmmmm ^= 0xE0;
      vex_XvvvvLpp ^= 0x78;

      EMIT_BYTE(kVex3Byte);
      EMIT_OP(vex_rxbmmmmm);
      EMIT_OP(vex_XvvvvLpp);
      EMIT_OP(opCode);

      rmReg &= 0x07;
    }
    else {
      vex_XvvvvLpp += static_cast<uint32_t>(opReg & 0x08) << 4;
      vex_XvvvvLpp ^= 0xF8;

      EMIT_BYTE(kVex2Byte);
      EMIT_OP(vex_XvvvvLpp);
      EMIT_OP(opCode);
    }
  }

  EMIT_BYTE(x86EncodeMod(3, opReg, static_cast<uint32_t>(rmReg)));

  if (imLen == 0)
    goto _EmitDone;

  EMIT_BYTE(imVal & 0xFF);
  goto _EmitDone;

_EmitAvxM:
  EMIT_AVX_M
  goto _EmitSib;

_EmitAvxV:
  EMIT_AVX_M

  if (mIndex >= kInvalidReg)
    goto _IllegalInst;

  if (Arch == kArchX64)
    mIndex &= 0x7;

  dispOffset = rmMem->getDisplacement();
  if (rmMem->isBaseIndexType()) {
    uint32_t shift = rmMem->getShift();

    if (mBase != kX86RegIndexBp && dispOffset == 0) {
      // [Base + Index * Scale].
      EMIT_BYTE(x86EncodeMod(0, opReg, 4));
      EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
    }
    else if (IntUtil::isInt8(dispOffset)) {
      // [Base + Index * Scale + Disp8].
      EMIT_BYTE(x86EncodeMod(1, opReg, 4));
      EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
      EMIT_BYTE(static_cast<int8_t>(dispOffset));
    }
    else {
      // [Base + Index * Scale + Disp32].
      EMIT_BYTE(x86EncodeMod(2, opReg, 4));
      EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
  }
  else {
    // [Index * Scale + Disp32].
    uint32_t shift = rmMem->getShift();

    EMIT_BYTE(x86EncodeMod(0, opReg, 4));
    EMIT_BYTE(x86EncodeSib(shift, mIndex, 5));

    if (rmMem->getMemType() == kMemTypeLabel) {
      if (Arch == kArchX64)
        goto _IllegalAddr;

      // Relative->Absolute [x86 mode].
      label = self->getLabelData(rmMem->_vmem.base);
      relocId = self->_relocList.getLength();

      RelocData reloc;
      reloc.type = kRelocRelToAbs;
      reloc.size = 4;
      reloc.from = static_cast<Ptr>((uintptr_t)(cursor - self->_buffer));
      reloc.data = static_cast<SignedPtr>(dispOffset);

      if (self->_relocList.append(reloc) != kErrorOk)
        return self->setError(kErrorNoHeapMemory);

      if (label->offset != -1) {
        // Bound label.
        reloc.data += static_cast<SignedPtr>(label->offset);
        EMIT_DWORD(0);
      }
      else {
        // Non-bound label.
        dispOffset = -4 - imLen;
        dispSize = 4;
        goto _EmitDisplacement;
      }
    }
    else {
      // [Disp32].
      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
  }
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Xop]
  // --------------------------------------------------------------------------

#define EMIT_XOP_M \
  ASMJIT_ASSERT(rmMem != NULL); \
  ASMJIT_ASSERT(rmMem->getOp() == kOperandTypeMem); \
  \
  if (rmMem->hasSegment()) { \
    EMIT_BYTE(x86SegmentPrefix[rmMem->getSegment()]); \
  } \
  \
  mBase = rmMem->getBase(); \
  mIndex = rmMem->getIndex(); \
  \
  { \
    uint32_t vex_XvvvvLpp; \
    uint32_t vex_rxbmmmmm; \
    \
    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift)) & 0x03; \
    vex_XvvvvLpp += (opX >> (kVexVVVVShift - 3)); \
    vex_XvvvvLpp += (opX << 4) & 0x80; \
    \
    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x1F; \
    vex_rxbmmmmm += static_cast<uint32_t>(mBase  - 8 < 8) << 5; \
    vex_rxbmmmmm += static_cast<uint32_t>(mIndex - 8 < 8) << 6; \
    \
    vex_rxbmmmmm |= static_cast<uint32_t>(opReg << 4) & 0x80; \
    vex_rxbmmmmm ^= 0xE0; \
    vex_XvvvvLpp ^= 0x78; \
    \
    EMIT_BYTE(kXopByte); \
    EMIT_BYTE(vex_rxbmmmmm); \
    EMIT_BYTE(vex_XvvvvLpp); \
    EMIT_OP(opCode); \
  } \
  \
  mBase &= 0x7; \
  opReg &= 0x7;

_EmitXopR:
  {
    uint32_t xop_XvvvvLpp;
    uint32_t xop_rxbmmmmm;

    xop_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    xop_XvvvvLpp |= (opCode >> (kX86InstOpCode_PP_Shift));
    xop_XvvvvLpp |= (opX    >> (kVexVVVVShift - 3));
    xop_XvvvvLpp |= (opX << 4) & 0x80;

    xop_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x1F;
    xop_rxbmmmmm |= (rmReg << 2) & 0x20;

    xop_rxbmmmmm |= static_cast<uint32_t>(opReg & 0x08) << 4;
    xop_rxbmmmmm ^= 0xE0;
    xop_XvvvvLpp ^= 0x78;

    EMIT_BYTE(kXopByte);
    EMIT_OP(xop_rxbmmmmm);
    EMIT_OP(xop_XvvvvLpp);
    EMIT_OP(opCode);

    rmReg &= 0x07;
  }

  EMIT_BYTE(x86EncodeMod(3, opReg, static_cast<uint32_t>(rmReg)));

  if (imLen == 0)
    goto _EmitDone;

  EMIT_BYTE(imVal & 0xFF);
  goto _EmitDone;

_EmitXopM:
  EMIT_XOP_M
  goto _EmitSib;

  // --------------------------------------------------------------------------
  // [Emit - Jump/Call to an Immediate]
  // --------------------------------------------------------------------------

  // 64-bit mode requires a trampoline if a relative displacement doesn't fit
  // into a 32-bit address. Old version of AsmJit used to emit jump to a section
  // which contained another jump followed by an address (it worked well for
  // both `jmp` and `call`), but it required to reserve 14-bytes for a possible
  // trampoline.
  //
  // Instead of using 5-byte `jmp/call` and reserving 14 bytes required by the
  // trampoline, it's better to use 6-byte `jmp/call` (prefixing it with REX
  // prefix) and to patch the `jmp/call` instruction to read the address from
  // a memory in case the trampoline is needed.
  //
_EmitJmpOrCallAbs:
  {
    RelocData rd;
    rd.type = kRelocAbsToRel;
    rd.size = 4;
    rd.from = (intptr_t)(cursor - self->_buffer) + 1;
    rd.data = static_cast<SignedPtr>(imVal);

    uint32_t trampolineSize = 0;

    if (Arch == kArchX64) {
      Ptr baseAddress = self->getBaseAddress();
      Ptr diff = rd.data - (baseAddress + rd.from + 4);

      // If the base address of the output is known, it's possible to determine
      // the need for a trampoline here. This saves possible REX prefix in
      // 64-bit mode and prevents reserving space needed for an absolute address.
      if (baseAddress == kNoBaseAddress || !x64IsRelative(rd.data, baseAddress + rd.from + 4)) {
        // Emit REX prefix so the instruction can be patched later on. The REX
        // prefix does nothing if not patched after, but allows to patch the
        // instruction in case where the trampoline is needed.
        rd.type = kRelocTrampoline;
        rd.from++;

        EMIT_OP(0x40);
        trampolineSize = 8;
      }
    }

    // Both `jmp` and `call` instructions have a single-byte opcode and are
    // followed by a 32-bit displacement.
    EMIT_OP(opCode);
    EMIT_DWORD(0);

    if (self->_relocList.append(rd) != kErrorOk)
      return self->setError(kErrorNoHeapMemory);

    // Reserve space for a possible trampoline.
    self->_trampolineSize += trampolineSize;
  }
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Displacement]
  // --------------------------------------------------------------------------

_EmitDisplacement:
  {
    ASMJIT_ASSERT(label->offset == -1);
    ASMJIT_ASSERT(dispSize == 1 || dispSize == 4);

    // Chain with label.
    LabelLink* link = self->_newLabelLink();
    link->prev = label->links;
    link->offset = (intptr_t)(cursor - self->_buffer);
    link->displacement = dispOffset;
    link->relocId = relocId;
    label->links = link;

    // Emit label size as dummy data.
    if (dispSize == 1)
      EMIT_BYTE(0x01);
    else // if (dispSize == 4)
      EMIT_DWORD(0x04040404);
  }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

_EmitDone:
#if !defined(ASMJIT_DISABLE_LOGGER)
# if defined(ASMJIT_DEBUG)
  if (self->_logger || assertIllegal) {
# else
  if (self->_logger) {
# endif // ASMJIT_DEBUG
    StringBuilderT<512> sb;
    uint32_t loggerOptions = 0;

    if (self->_logger) {
      sb.appendString(self->_logger->getIndentation());
      loggerOptions = self->_logger->getOptions();
    }

    X86Assembler_dumpInstruction(sb, Arch, code, options, o0, o1, o2, o3, loggerOptions);

    if ((loggerOptions & (1 << kLoggerOptionBinaryForm)) != 0)
      X86Assembler_dumpComment(sb, sb.getLength(), self->_cursor, (intptr_t)(cursor - self->_cursor), dispSize, self->_comment);
    else
      X86Assembler_dumpComment(sb, sb.getLength(), NULL, 0, 0, self->_comment);

# if defined(ASMJIT_DEBUG)
    if (self->_logger)
# endif // ASMJIT_DEBUG
      self->_logger->logString(kLoggerStyleDefault, sb.getData(), sb.getLength());

# if defined(ASMJIT_DEBUG)
    // Raise an assertion failure, because this situation shouldn't happen.
    if (assertIllegal)
      assertionFailed(sb.getData(), __FILE__, __LINE__);
# endif // ASMJIT_DEBUG
  }
#else
# if defined(ASMJIT_DEBUG)
  ASMJIT_ASSERT(!assertIllegal);
# endif // ASMJIT_DEBUG
#endif // !ASMJIT_DISABLE_LOGGER

  self->_comment = NULL;
  self->setCursor(cursor);

  return kErrorOk;

_GrowBuffer:
  ASMJIT_PROPAGATE_ERROR(self->_grow(16));

  cursor = self->getCursor();
  goto _Prepare;
}

Error X86Assembler::_emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) {
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_BUILD_X64)
  return X86Assembler_emit<kArchX86>(this, code, &o0, &o1, &o2, &o3);
#elif !defined(ASMJIT_BUILD_X86) && defined(ASMJIT_BUILD_X64)
  return X86Assembler_emit<kArchX64>(this, code, &o0, &o1, &o2, &o3);
#else
  if (_arch == kArchX86)
    return X86Assembler_emit<kArchX86>(this, code, &o0, &o1, &o2, &o3);
  else
    return X86Assembler_emit<kArchX64>(this, code, &o0, &o1, &o2, &o3);
#endif
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
