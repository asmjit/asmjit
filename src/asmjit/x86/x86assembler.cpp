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
#include "../base/containers.h"
#include "../base/cpuinfo.h"
#include "../base/logger.h"
#include "../base/runtime.h"
#include "../base/utils.h"
#include "../base/vmem.h"
#include "../x86/x86assembler.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Constants]
// ============================================================================

enum { kX86RexNoRexMask = kX86InstOptionRex | _kX86InstOptionNoRex };

//! \internal
//!
//! X86/X64 bytes used to encode important prefixes.
enum X86Byte {
  //! 1-byte REX prefix
  kX86ByteRex = 0x40,

  //! 1-byte REX.W component.
  kX86ByteRexW = 0x08,

  //! 2-byte VEX prefix:
  //!   - `[0]` - `0xC5`.
  //!   - `[1]` - `RvvvvLpp`.
  kX86ByteVex2 = 0xC5,

  //! 3-byte VEX prefix.
  //!   - `[0]` - `0xC4`.
  //!   - `[1]` - `RXBmmmmm`.
  //!   - `[2]` - `WvvvvLpp`.
  kX86ByteVex3 = 0xC4,

  //! 3-byte XOP prefix.
  //!   - `[0]` - `0x8F`.
  //!   - `[1]` - `RXBmmmmm`.
  //!   - `[2]` - `WvvvvLpp`.
  kX86ByteXop3 = 0x8F,

  //! 4-byte EVEX prefix.
  //!   - `[0]` - `0x62`.
  //!   - `[1]` - Payload0 or `P[ 7: 0]` - `[R  X  B  R' 0  0  m  m]`.
  //!   - `[2]` - Payload1 or `P[15: 8]` - `[W  v  v  v  v  1  p  p]`.
  //!   - `[3]` - Payload2 or `P[23:16]` - `[z  L' L  b  V' a  a  a]`.
  //!
  //! Groups:
  //!   - `P[ 1: 0]` - EXT: VEX.mmmmm, only lowest 2 bits used.
  //!   - `P[ 3: 2]` - ___: Must be 0.
  //!   - `P[    4]` - REG: EVEX.R'.
  //!   - `P[    5]` - REG: EVEX.B.
  //!   - `P[    6]` - REG: EVEX.X.
  //!   - `P[    7]` - REG: EVEX.R.
  //!   - `P[ 9: 8]` - EXT: VEX.pp.
  //!   - `P[   10]` - ___: Must be 1.
  //!   - `P[14:11]` - REG: 2nd SRC vector register (4 bits).
  //!   - `P[   15]` - EXT: VEX.W.
  //!   - `P[18:16]` - REG: K registers k0...k7 (Merging/Zeroing Vector Ops.).
  //!   - `P[   19]` - REG: 2nd SRC vector register (Hi bit).
  //!   - `P[   20]` - EXT: Broadcast/Static-Rounding/SAE bit.
  //!   - `P[22.21]` - EXT: Vector Length/Rounding Control.
  //!   - `P[   23]` - EXT: Destination result behavior (Merging/Zeroing Vector Ops.).
  kX86ByteEvex4 = 0x62
};

// AsmJit specific (used to encode VVVV field in XOP/VEX).
enum VexVVVV {
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
//! Mandatory prefixes encoded in 'asmjit' opcode [66, F3, F2] and AsmJit
//! extensions
static const uint8_t x86OpCodePP[8] = {
  0x00, 0x66, 0xF3, 0xF2, 0x00, 0x00, 0x00, 0x9B
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

static ASMJIT_INLINE uint32_t x86RexFromOpCodeAndOptions(uint32_t opCode, uint32_t options) {
  uint32_t rex = (opCode >> (kX86InstOpCode_W_Shift - 3));
  ASMJIT_ASSERT((rex & ~static_cast<uint32_t>(0x08)) == 0);

  return rex + (options & kX86RexNoRexMask);
}

static ASMJIT_INLINE bool x86RexIsInvalid(uint32_t rex) {
  return rex >= _kX86InstOptionNoRex;
}

//! Encode ModR/M.
static ASMJIT_INLINE uint32_t x86EncodeMod(uint32_t m, uint32_t o, uint32_t rm) {
  ASMJIT_ASSERT(m <= 3);
  ASMJIT_ASSERT(o <= 7);
  ASMJIT_ASSERT(rm <= 7);
  return (m << 6) + (o << 3) + rm;
}

//! Encode SIB.
static ASMJIT_INLINE uint32_t x86EncodeSib(uint32_t s, uint32_t i, uint32_t b) {
  ASMJIT_ASSERT(s <= 3);
  ASMJIT_ASSERT(i <= 7);
  ASMJIT_ASSERT(b <= 7);
  return (s << 6) + (i << 3) + b;
}

//! Get whether the two pointers `a` and `b` can be encoded by using relative
//! displacement, which fits into a signed 32-bit integer.
static ASMJIT_INLINE bool x64IsRelative(Ptr a, Ptr b) {
  SignedPtr diff = static_cast<SignedPtr>(a) - static_cast<SignedPtr>(b);
  return Utils::isInt32(diff);
}

//! Cast `reg` to `X86Reg` and get the register index.
static ASMJIT_INLINE uint32_t x86OpReg(const Operand* reg) {
  return static_cast<const X86Reg*>(reg)->getRegIndex();
}

//! Cast `mem` to `X86Mem` and return it.
static ASMJIT_INLINE const X86Mem* x86OpMem(const Operand* mem) {
  return static_cast<const X86Mem*>(mem);
}

//! Combine `regIndex` and `vvvvIndex` into single value (used by AVX and AVX-512).
static ASMJIT_INLINE uint32_t x86RegAndVvvv(uint32_t regIndex, uint32_t vvvvIndex) {
  return regIndex + (vvvvIndex << kVexVVVVShift);
}

//! Get `O` field of `opCode`.
static ASMJIT_INLINE uint32_t x86ExtractO(uint32_t opCode) {
  return (opCode >> kX86InstOpCode_O_Shift) & 0x07;
}

static ASMJIT_INLINE bool x86IsXmm(const Operand* op) { return op->isRegType(kX86RegTypeXmm); }
static ASMJIT_INLINE bool x86IsXmm(const X86Reg* reg) { return reg->isXmm(); }

static ASMJIT_INLINE bool x86IsYmm(const Operand* op) { return op->isRegType(kX86RegTypeYmm); }
static ASMJIT_INLINE bool x86IsYmm(const X86Reg* reg) { return reg->isYmm(); }

// ============================================================================
// [Macros]
// ============================================================================

#define ENC_OPS(op0, op1, op2) \
  ((Operand::kType##op0) + ((Operand::kType##op1) << 3) + ((Operand::kType##op2) << 6))

#define ADD_66H_P(exp) \
  do { \
    opCode |= (static_cast<uint32_t>(exp) << kX86InstOpCode_PP_Shift); \
  } while (0)

#define ADD_66H_P_BY_SIZE(sz) \
  do { \
    opCode |= (static_cast<uint32_t>(sz) & 0x02) << (kX86InstOpCode_PP_Shift - 1); \
  } while (0)

#define ADD_REX_W(exp) \
  do { \
    if (Arch == kArchX64) \
      opCode |= static_cast<uint32_t>(exp) << kX86InstOpCode_W_Shift; \
  } while (0)

#define ADD_REX_W_BY_SIZE(sz) \
  do { \
    if (Arch == kArchX64 && (sz) == 8) \
      opCode |= kX86InstOpCode_W; \
  } while (0)

#define ADD_VEX_W(exp) \
  do { \
    opCode |= static_cast<uint32_t>(exp) << kX86InstOpCode_W_Shift; \
  } while (0)

#define ADD_VEX_L(exp) \
  do { \
    opCode |= static_cast<uint32_t>(exp) << kX86InstOpCode_L_Shift; \
  } while (0)

#define EMIT_BYTE(_Val_) \
  do { \
    cursor[0] = static_cast<uint8_t>((_Val_) & 0xFF); \
    cursor += 1; \
  } while (0)

#define EMIT_WORD(_Val_) \
  do { \
    Utils::writeU16uLE(cursor, static_cast<uint32_t>(_Val_)); \
    cursor += 2; \
  } while (0)

#define EMIT_DWORD(_Val_) \
  do { \
    Utils::writeU32uLE(cursor, static_cast<uint32_t>(_Val_)); \
    cursor += 4; \
  } while (0)

#define EMIT_QWORD(_Val_) \
  do { \
    Utils::writeU64uLE(cursor, static_cast<uint64_t>(_Val_)); \
    cursor += 8; \
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

X86Assembler::X86Assembler(Runtime* runtime, uint32_t arch)
  : Assembler(runtime),
    zax(NoInit),
    zcx(NoInit),
    zdx(NoInit),
    zbx(NoInit),
    zsp(NoInit),
    zbp(NoInit),
    zsi(NoInit),
    zdi(NoInit) {

  ASMJIT_ASSERT(arch == kArchX86 || arch == kArchX64);
  _setArch(arch);
}

X86Assembler::~X86Assembler() {}

// ============================================================================
// [asmjit::X86Assembler - Arch]
// ============================================================================

Error X86Assembler::_setArch(uint32_t arch) {
#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86) {
    _arch = arch;
    _regSize = 4;

    _regCount.reset();
    _regCount._gp  = 8;
    _regCount._mm  = 8;
    _regCount._k   = 8;
    _regCount._xyz = 8;
    ::memcpy(&zax, &x86RegData.gpd, sizeof(Operand) * 8);

    return kErrorOk;
  }
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64) {
    _arch = arch;
    _regSize = 8;

    _regCount.reset();
    _regCount._gp  = 16;
    _regCount._mm  = 8;
    _regCount._k   = 8;
    _regCount._xyz = 16;
    ::memcpy(&zax, &x86RegData.gpq, sizeof(Operand) * 8);

    return kErrorOk;
  }
#endif // ASMJIT_BUILD_X64

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
  RelocData rd;

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger)
    _logger->logFormat(Logger::kStyleData, regSize == 4 ? ".dd L%u\n" : ".dq L%u\n", op.getId());
#endif // !ASMJIT_DISABLE_LOGGER

  rd.type = kRelocRelToAbs;
  rd.size = regSize;
  rd.from = static_cast<Ptr>(getOffset());
  rd.data = 0;

  if (label->offset != -1) {
    // Bound label.
    rd.data = static_cast<Ptr>(static_cast<SignedPtr>(label->offset));
  }
  else {
    // Non-bound label. Need to chain.
    LabelLink* link = _newLabelLink();

    link->prev = (LabelLink*)label->links;
    link->offset = getOffset();
    link->displacement = 0;
    link->relocId = _relocations.getLength();

    label->links = link;
  }

  if (_relocations.append(rd) != kErrorOk)
    return setLastError(kErrorNoHeapMemory);

  // Emit dummy intptr_t (4 or 8 bytes; depends on the address size).
  EMIT_DWORD(0);
  if (regSize == 8)
    EMIT_DWORD(0);

  setCursor(cursor);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Assembler - Align]
// ============================================================================

Error X86Assembler::align(uint32_t alignMode, uint32_t offset) noexcept {
#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger)
    _logger->logFormat(Logger::kStyleDirective,
      "%s.align %u\n", _logger->getIndentation(), static_cast<unsigned int>(offset));
#endif // !ASMJIT_DISABLE_LOGGER

  if (alignMode > kAlignZero)
    return setLastError(kErrorInvalidArgument);

  if (offset <= 1)
    return kErrorOk;

  if (!Utils::isPowerOf2(offset) || offset > 64)
    return setLastError(kErrorInvalidArgument);

  uint32_t i = static_cast<uint32_t>(Utils::alignDiff<size_t>(getOffset(), offset));
  if (i == 0)
    return kErrorOk;

  if (getRemainingSpace() < i)
    ASMJIT_PROPAGATE_ERROR(_grow(i));

  uint8_t* cursor = getCursor();
  uint8_t pattern = 0x00;

  switch (alignMode) {
    case kAlignCode: {
      if (hasAsmOption(kOptionOptimizedAlign)) {
        // Intel 64 and IA-32 Architectures Software Developer's Manual - Volume 2B (NOP).
        enum { kMaxNopSize = 9 };

        static const uint8_t nopData[kMaxNopSize][kMaxNopSize] = {
          { 0x90 },
          { 0x66, 0x90 },
          { 0x0F, 0x1F, 0x00 },
          { 0x0F, 0x1F, 0x40, 0x00 },
          { 0x0F, 0x1F, 0x44, 0x00, 0x00 },
          { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 },
          { 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 },
          { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
          { 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }
        };

        do {
          uint32_t n = Utils::iMin<uint32_t>(i, kMaxNopSize);
          const uint8_t* p = nopData[n - 1];

          i -= n;
          do {
            EMIT_BYTE(*p++);
          } while (--n);
        } while (i);
      }

      pattern = 0x90;
      break;
    }

    case kAlignData: {
      pattern = 0xCC;
      break;
    }

    case kAlignZero: {
      // Already set to zero.
      break;
    }
  }

  while (i) {
    EMIT_BYTE(pattern);
    i--;
  }

  setCursor(cursor);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Assembler - Reloc]
// ============================================================================

size_t X86Assembler::_relocCode(void* _dst, Ptr baseAddress) const noexcept {
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
  size_t relocCount = _relocations.getLength();
  const RelocData* rdList = _relocations.getData();

  for (size_t i = 0; i < relocCount; i++) {
    const RelocData& rd = rdList[i];

    // Make sure that the `RelocData` is correct.
    Ptr ptr = rd.data;

    size_t offset = static_cast<size_t>(rd.from);
    ASMJIT_ASSERT(offset + rd.size <= static_cast<Ptr>(maxCodeSize));

    // Whether to use trampoline, can be only used if relocation type is
    // kRelocAbsToRel on 64-bit.
    bool useTrampoline = false;

    switch (rd.type) {
      case kRelocAbsToAbs:
        break;

      case kRelocRelToAbs:
        ptr += baseAddress;
        break;

      case kRelocAbsToRel:
        ptr -= baseAddress + rd.from + 4;
        break;

      case kRelocTrampoline:
        ptr -= baseAddress + rd.from + 4;
        if (!Utils::isInt32(static_cast<SignedPtr>(ptr))) {
          ptr = (Ptr)tramp - (baseAddress + rd.from + 4);
          useTrampoline = true;
        }
        break;

      default:
        ASMJIT_NOT_REACHED();
    }

    switch (rd.size) {
      case 4:
        Utils::writeU32u(dst + offset, static_cast<int32_t>(static_cast<SignedPtr>(ptr)));
        break;

      case 8:
        Utils::writeI64u(dst + offset, static_cast<int64_t>(ptr));
        break;

      default:
        ASMJIT_NOT_REACHED();
    }

    // Handle the trampoline case.
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
      Utils::writeU64u(tramp, static_cast<uint64_t>(rd.data));

      // Advance trampoline pointer.
      tramp += 8;

#if !defined(ASMJIT_DISABLE_LOGGER)
      if (logger)
        logger->logFormat(Logger::kStyleComment, "; Trampoline %llX\n", rd.data);
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
static const char* AssemblerX86_getAddressSizeString(uint32_t size) noexcept {
  switch (size) {
    case 1 : return "byte ptr ";
    case 2 : return "word ptr ";
    case 4 : return "dword ptr ";
    case 8 : return "qword ptr ";
    case 10: return "tword ptr ";
    case 16: return "oword ptr ";
    case 32: return "yword ptr ";
    case 64: return "zword ptr ";
    default: return "";
  }
}

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
      ASMJIT_FALLTHROUGH;

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

    case kX86RegTypeK:
      sb._appendString("k", 1);
      goto _EmitID;

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

    case kX86RegTypeZmm:
      sb._appendString("zmm", 3);
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

    sb._appendString(AssemblerX86_getAddressSizeString(op->getSize()));

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

      case kMemTypeRip:
        // [rip + displacement]
        sb.appendString("rip", 3);
        break;

      default:
        sb.appendFormat("<invalid %d>", m->getMemType());
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
      if ((loggerOptions & Logger::kOptionHexDisplacement) != 0 && dispOffset > 9) {
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

    if ((loggerOptions & Logger::kOptionHexImmediate) != 0 && static_cast<uint64_t>(val) > 9)
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
  uint32_t code,
  uint32_t options,
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
#endif // !ASMJIT_DISABLE_LOGGER

// ============================================================================
// [asmjit::X86Assembler - Emit]
// ============================================================================

#define HI_REG(_Index_) ((_kX86RegTypePatchedGpbHi << 8) | _Index_)
//! \internal
static const Operand::VRegOp x86PatchedHiRegs[4] = {
  // ----------------+---+--------------+--------------+------------+
  // Operand         | S | Reg. Code    | OperandId    |   Unused   |
  // ----------------+---+--------------+--------------+------------+
  { Operand::kTypeReg, 1 , { HI_REG(4) }, kInvalidValue, {{ 0, 0 }} },
  { Operand::kTypeReg, 1 , { HI_REG(5) }, kInvalidValue, {{ 0, 0 }} },
  { Operand::kTypeReg, 1 , { HI_REG(6) }, kInvalidValue, {{ 0, 0 }} },
  { Operand::kTypeReg, 1 , { HI_REG(7) }, kInvalidValue, {{ 0, 0 }} }
};
#undef HI_REG

template<int Arch>
static ASMJIT_INLINE Error X86Assembler_emit(Assembler* self_, uint32_t code, const Operand* o0, const Operand* o1, const Operand* o2, const Operand* o3) {
  X86Assembler* self = static_cast<X86Assembler*>(self_);
  uint32_t options = self->getInstOptionsAndReset();

  // Invalid instruction.
  if (code >= _kX86InstIdCount) {
    self->_comment = nullptr;
    return self->setLastError(kErrorUnknownInst);
  }

  // --------------------------------------------------------------------------
  // [Grow]
  // --------------------------------------------------------------------------

  // Grow request happens rarely.
  uint8_t* cursor = self->getCursor();
  if (ASMJIT_UNLIKELY((size_t)(self->_end - cursor) < 16)) {
    ASMJIT_PROPAGATE_ERROR(self->_grow(16));
    cursor = self->getCursor();
  }

  // --------------------------------------------------------------------------
  // [Prepare]
  // --------------------------------------------------------------------------

  uint32_t encoded = o0->getOp() + (o1->getOp() << 3) + (o2->getOp() << 6);

  // Instruction opcode.
  uint32_t opCode;
  // ModR/M opcode or register code.
  uint32_t opReg;

  // ModR/M, both rmReg and rmMem should refer to the same variable since they
  // are never used together - either `rmReg` or `rmMem`.
  union {
    // ModR/M - register code.
    uintptr_t rmReg;
    // ModR/M - Memory operand.
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

  opCode = info.getPrimaryOpCode();
  opReg = x86ExtractO(opCode);

  if (Arch == kArchX86) {
    // Check if one or more register operand is one of AH, BH, CH, or DH and
    // patch them to ensure that the binary code with correct byte-index (4-7)
    // is generated.
    if (o0->isRegType(kX86RegTypeGpbHi))
      o0 = (const Operand*)(&x86PatchedHiRegs[x86OpReg(o0)]);

    if (o1->isRegType(kX86RegTypeGpbHi))
      o1 = (const Operand*)(&x86PatchedHiRegs[x86OpReg(o1)]);
  }
  else {
    // `W` field.
    ASMJIT_ASSERT(static_cast<uint32_t>(kX86InstOptionRex) == static_cast<uint32_t>(kX86ByteRex));

    // Check if one or more register operand is one of BPL, SPL, SIL, DIL and
    // force a REX prefix to be emitted in such case.
    if (X86Reg::isGpbReg(*o0)) {
      uint32_t index = x86OpReg(o0);
      if (static_cast<const X86Reg*>(o0)->isGpbLo()) {
        options |= (index >= 4) ? kX86InstOptionRex : 0;
      }
      else {
        options |= _kX86InstOptionNoRex;
        o0 = reinterpret_cast<const Operand*>(&x86PatchedHiRegs[index]);
      }
    }

    if (X86Reg::isGpbReg(*o1)) {
      uint32_t index = x86OpReg(o1);
      if (static_cast<const X86Reg*>(o1)->isGpbLo()) {
        options |= (index >= 4) ? kX86InstOptionRex : 0;
      }
      else {
        options |= _kX86InstOptionNoRex;
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

  switch (info.getEncodingId()) {
    // ------------------------------------------------------------------------
    // [None]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdNone:
      goto _EmitDone;

    // ------------------------------------------------------------------------
    // [X86]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdX86Op_66H:
      ADD_66H_P(true);
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdX86Op:
      goto _EmitX86Op;

    case kX86InstEncodingIdX86Rm_B:
      opCode += o0->getSize() != 1;
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdX86Rm:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86RmReg:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86RegRm:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        if (o0->getSize() == 1 || o0->getSize() != o1->getSize())
          goto _IllegalInst;

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        if (o0->getSize() == 1)
          goto _IllegalInst;

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86M:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Arith:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode += (o0->getSize() != 1) + 2;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode += (o0->getSize() != 1) + 2;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      // The remaining instructions use 0x80 opcode.
      opCode = 0x80;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = Utils::isInt8(imVal) ? static_cast<uint32_t>(1) : Utils::iMin<uint32_t>(o0->getSize(), 4);
        rmReg = x86OpReg(o0);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Alternate Form - AL, AX, EAX, RAX.
        if (rmReg == 0 && (o0->getSize() == 1 || imLen != 1)) {
          opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
          opCode |= ((opReg << 3) | (0x04 + (o0->getSize() != 1)));
          imLen = Utils::iMin<uint32_t>(o0->getSize(), 4);
          goto _EmitX86Op;
        }

        opCode += o0->getSize() != 1 ? (imLen != 1 ? 1 : 3) : 0;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        uint32_t memSize = o0->getSize();

        if (memSize == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = Utils::isInt8(imVal) ? static_cast<uint32_t>(1) : Utils::iMin<uint32_t>(memSize, 4);

        opCode += memSize != 1 ? (imLen != 1 ? 1 : 3) : 0;
        ADD_66H_P_BY_SIZE(memSize);
        ADD_REX_W_BY_SIZE(memSize);

        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86BSwap:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (o0->getSize() < 4)
          goto _IllegalInst;

        opReg = x86OpReg(o0);
        ADD_REX_W_BY_SIZE(o0->getSize());
        goto _EmitX86OpWithOpReg;
      }
      break;

    case kX86InstEncodingIdX86BTest:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      // The remaining instructions use the secondary opcode/r.
      imVal = static_cast<const Imm*>(o1)->getInt64();
      imLen = 1;

      opCode = extendedInfo.getSecondaryOpCode();
      opReg = x86ExtractO(opCode);

      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Call:
      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
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
          EMIT_BYTE(opCode);
          EMIT_DWORD(static_cast<int32_t>(offs - kRel32Size));
        }
        else {
          // Non-bound label.
          EMIT_BYTE(opCode);
          dispOffset = -4;
          dispSize = 4;
          relocId = -1;
          goto _EmitDisplacement;
        }
        goto _EmitDone;
      }
      break;

    case kX86InstEncodingIdX86Enter:
      if (encoded == ENC_OPS(Imm, Imm, None)) {
        EMIT_BYTE(0xC8);
        EMIT_WORD(static_cast<const Imm*>(o1)->getUInt16());
        EMIT_BYTE(static_cast<const Imm*>(o0)->getUInt8());
        goto _EmitDone;
      }
      break;

    case kX86InstEncodingIdX86Imul:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
        opCode |= 0xF6 + (o0->getSize() != 1);

        opReg = 5;
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
        opCode |= 0xF6 + (o0->getSize() != 1);

        opReg = 5;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      // The following instructions use 0x0FAF opcode.
      opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
      opCode |= kX86InstOpCode_MM_0F | 0xAF;

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);

        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);

        goto _EmitX86M;
      }

      // The following instructions use 0x69/0x6B opcode.
      opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
      opCode |= 0x6B;

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        if (!Utils::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = x86OpReg(o0);
        rmReg = opReg;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        if (!Utils::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        ASMJIT_ASSERT(o0->getSize() != 1);

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        if (!Utils::isInt8(imVal)) {
          opCode -= 2;
          imLen = o0->getSize() == 2 ? 2 : 4;
        }

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86IncDec:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = x86OpReg(o0);

        // INC r16|r32 is not encodable in 64-bit mode.
        if (Arch == kArchX86 && (o0->getSize() == 2 || o0->getSize() == 4)) {
          opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
          opCode |= extendedInfo.getSecondaryOpCode() + (static_cast<uint32_t>(rmReg) & 0x07);
          goto _EmitX86Op;
        }
        else {
          opCode += o0->getSize() != 1;
          goto _EmitX86R;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        opCode += o0->getSize() != 1;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Int:
      if (encoded == ENC_OPS(Imm, None, None)) {
        imVal = static_cast<const Imm*>(o0)->getInt64();
        uint8_t imm8 = static_cast<uint8_t>(imVal & 0xFF);

        if (imm8 == 0x03) {
          EMIT_BYTE(opCode);
        }
        else {
          EMIT_BYTE(opCode + 1);
          EMIT_BYTE(imm8);
        }
        goto _EmitDone;
      }
      break;

    case kX86InstEncodingIdX86Jcc:
      if (encoded == ENC_OPS(Label, None, None)) {
        label = self->getLabelData(static_cast<const Label*>(o0)->getId());

        if (self->hasAsmOption(Assembler::kOptionPredictedJumps)) {
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

          if ((options & kInstOptionLongForm) == 0 && Utils::isInt8(offs - kRel8Size)) {
            EMIT_BYTE(opCode);
            EMIT_BYTE(offs - kRel8Size);

            options |= kInstOptionShortForm;
            goto _EmitDone;
          }
          else {
            EMIT_BYTE(0x0F);
            EMIT_BYTE(opCode + 0x10);
            EMIT_DWORD(static_cast<int32_t>(offs - kRel32Size));

            options &= ~kInstOptionShortForm;
            goto _EmitDone;
          }
        }
        else {
          // Non-bound label.
          if (options & kInstOptionShortForm) {
            EMIT_BYTE(opCode);
            dispOffset = -1;
            dispSize = 1;
            relocId = -1;
            goto _EmitDisplacement;
          }
          else {
            EMIT_BYTE(0x0F);
            EMIT_BYTE(opCode + 0x10);
            dispOffset = -4;
            dispSize = 4;
            relocId = -1;
            goto _EmitDisplacement;
          }
        }
      }
      break;

    case kX86InstEncodingIdX86Jecxz:
      if (encoded == ENC_OPS(Reg, Label, None)) {
        if (x86OpReg(o0) != kX86RegIndexCx)
          goto _IllegalInst;

        if ((Arch == kArchX86 && o0->getSize() == 2) ||
            (Arch == kArchX64 && o0->getSize() == 4)) {
          EMIT_BYTE(0x67);
        }

        EMIT_BYTE(0xE3);
        label = self->getLabelData(static_cast<const Label*>(o1)->getId());

        if (label->offset != -1) {
          // Bound label.
          intptr_t offs = label->offset - (intptr_t)(cursor - self->_buffer) - 1;
          if (!Utils::isInt8(offs))
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

    case kX86InstEncodingIdX86Jmp:
      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
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

          if ((options & kInstOptionLongForm) == 0 && Utils::isInt8(offs - kRel8Size)) {
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

    case kX86InstEncodingIdX86Lea:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Mov:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);

        // Asmjit uses segment registers indexed from 1 to 6, leaving zero as
        // "no segment register used". We have to fix this (decrement the index
        // of the register) when emitting MOV instructions which move to/from
        // a segment register. The segment register is always `opReg`, because
        // the MOV instruction uses RM or MR encoding.

        // Sreg <- Reg
        if (static_cast<const X86Reg*>(o0)->isSeg()) {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isGpw() ||
                        static_cast<const X86Reg*>(o1)->isGpd() ||
                        static_cast<const X86Reg*>(o1)->isGpq() );

          // `opReg` is the segment register.
          opReg--;
          opCode = 0x8E;

          ADD_66H_P_BY_SIZE(o1->getSize());
          ADD_REX_W_BY_SIZE(o1->getSize());
          goto _EmitX86R;
        }
        // Reg <- Sreg
        else if (static_cast<const X86Reg*>(o1)->isSeg()) {
          ASMJIT_ASSERT(static_cast<const X86Reg*>(o0)->isGpw() ||
                        static_cast<const X86Reg*>(o0)->isGpd() ||
                        static_cast<const X86Reg*>(o0)->isGpq() );

          // `opReg` is the segment register.
          opReg = static_cast<uint32_t>(rmReg) - 1;
          rmReg = x86OpReg(o0);
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
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);

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
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);

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
        rmReg = x86OpReg(o0);

        // Optimize instruction size by using 32-bit immediate if possible.
        if (Arch == kArchX64 && imLen == 8 && Utils::isInt32(imVal)) {
          opCode = 0xC7;
          ADD_REX_W(1);
          imLen = 4;
          goto _EmitX86R;
        }
        else {
          opCode = 0xB0 + (static_cast<uint32_t>(o0->getSize() != 1) << 3);
          opReg = rmReg;

          ADD_66H_P_BY_SIZE(imLen);
          ADD_REX_W_BY_SIZE(imLen);
          goto _EmitX86OpWithOpReg;
        }
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        uint32_t memSize = o0->getSize();

        if (memSize == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = Utils::iMin<uint32_t>(memSize, 4);

        opCode = 0xC6 + (memSize != 1);
        opReg = 0;
        ADD_66H_P_BY_SIZE(memSize);
        ADD_REX_W_BY_SIZE(memSize);

        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86MovSxZx:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86MovSxd:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_REX_W(true);

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_REX_W(true);

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86MovPtr:
      if (encoded == ENC_OPS(Reg, Imm, None)) {
        if (x86OpReg(o0) != 0)
          goto _IllegalInst;

        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = self->_regSize;
        goto _EmitX86Op;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Imm, Reg, None)) {
        if (x86OpReg(o1) != 0)
          goto _IllegalInst;

        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        imVal = static_cast<const Imm*>(o0)->getInt64();
        imLen = self->_regSize;
        goto _EmitX86Op;
      }
      break;

    case kX86InstEncodingIdX86Push:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (o0->isRegType(kX86RegTypeSeg)) {
          uint32_t segment = x86OpReg(o0);
          if (segment >= kX86SegCount)
            goto _IllegalInst;

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
        imLen = Utils::isInt8(imVal) ? 1 : 4;

        EMIT_BYTE(imLen == 1 ? 0x6A : 0x68);
        goto _EmitImm;
      }
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdX86Pop:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (o0->isRegType(kX86RegTypeSeg)) {
          uint32_t segment = x86OpReg(o0);
          if (segment == kX86SegCs || segment >= kX86SegCount)
            goto _IllegalInst;

          if (segment >= kX86SegFs)
            EMIT_BYTE(0x0F);

          EMIT_BYTE(x86OpCodePopSeg[segment]);
          goto _EmitDone;
        }
        else {
_GroupPop_Gp:
          // We allow 2 byte, 4 byte, and 8 byte register sizes, althought PUSH
          // and POP only allow 2 bytes or native size. On 64-bit we simply
          // PUSH/POP 64-bit register even if 32-bit register was given.
          if (o0->getSize() < 2)
            goto _IllegalInst;

          opCode = extendedInfo.getSecondaryOpCode();
          opReg = x86OpReg(o0);

          ADD_66H_P_BY_SIZE(o0->getSize());
          goto _EmitX86OpWithOpReg;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        if (o0->getSize() != 2 && o0->getSize() != self->getRegSize())
          goto _IllegalInst;

        ADD_66H_P_BY_SIZE(o0->getSize());
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Rep:
      // Emit REP 0xF2 or 0xF3 prefix first.
      EMIT_BYTE(0xF2 + opReg);
      goto _EmitX86Op;

    case kX86InstEncodingIdX86Ret:
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

    case kX86InstEncodingIdX86Rot:
      opCode += o0->getSize() != 1;
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        opCode += 2;
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o1)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        opCode += 2;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64() & 0xFF;
        imLen = imVal != 1;
        if (imLen)
          opCode -= 0x10;
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64() & 0xFF;
        imLen = imVal != 1;
        if (imLen)
          opCode -= 0x10;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Set:
      if (encoded == ENC_OPS(Reg, None, None)) {
        ASMJIT_ASSERT(o0->getSize() == 1);

        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        ASMJIT_ASSERT(o0->getSize() <= 1);

        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Shlrd:
      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      // The following instructions use opCode + 1.
      opCode++;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o2)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Reg)) {
        ASMJIT_ASSERT(static_cast<const X86Reg*>(o2)->isRegCode(kX86RegTypeGpbLo, kX86RegIndexCx));

        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Test:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ASMJIT_ASSERT(o0->getSize() == o1->getSize());

        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode() + (o0->getSize() != 1);
      opReg = x86ExtractO(opCode);

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = Utils::iMin<uint32_t>(o0->getSize(), 4);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Alternate Form - AL, AX, EAX, RAX.
        if (x86OpReg(o0) == 0) {
          opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
          opCode |= 0xA8 + (o0->getSize() != 1);
          goto _EmitX86Op;
        }

        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Imm, None)) {
        if (o0->getSize() == 0)
          goto _IllegalInst;

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = Utils::iMin<uint32_t>(o0->getSize(), 4);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdX86Xchg:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opCode += o0->getSize() != 1;
        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdX86Xadd:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        // Special opcode for 'xchg ?ax, reg'.
        if (code == kX86InstIdXchg && o0->getSize() > 1 && (opReg == 0 || rmReg == 0)) {
          opCode &= kX86InstOpCode_PP_66 | kX86InstOpCode_W;
          opCode |= 0x90;
          // One of `xchg a, b` or `xchg b, a` is AX/EAX/RAX.
          opReg += rmReg;
          goto _EmitX86OpWithOpReg;
        }

        opCode += o0->getSize() != 1;
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode += o1->getSize() != 1;
        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Fpu]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdFpuOp:
      goto _EmitFpuOp;

    case kX86InstEncodingIdFpuArith:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);

        // We switch to the alternative opcode if the first operand is zero.
        if (opReg == 0) {
_EmitFpArith_Reg:
          opCode = 0xD800 + ((opCode >> 8) & 0xFF) + static_cast<uint32_t>(rmReg);
          goto _EmitFpuOp;
        }
        else if (rmReg == 0) {
          rmReg = opReg;
          opCode = 0xDC00 + ((opCode >> 0) & 0xFF) + static_cast<uint32_t>(rmReg);
          goto _EmitFpuOp;
        }
        else {
          goto _IllegalInst;
        }
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        // 0xD8/0xDC, depends on the size of the memory operand; opReg has been
        // set already.
_EmitFpArith_Mem:
        opCode = (o0->getSize() == 4) ? 0xD8 : 0xDC;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdFpuCom:
      if (encoded == ENC_OPS(None, None, None)) {
        rmReg = 1;
        goto _EmitFpArith_Reg;
      }

      if (encoded == ENC_OPS(Reg, None, None)) {
        rmReg = x86OpReg(o0);
        goto _EmitFpArith_Reg;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        goto _EmitFpArith_Mem;
      }
      break;

    case kX86InstEncodingIdFpuFldFst:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);

        if (o0->getSize() == 4 && info.hasInstFlag(kX86InstFlagMem4)) {
          goto _EmitX86M;
        }

        if (o0->getSize() == 8 && info.hasInstFlag(kX86InstFlagMem8)) {
          opCode += 4;
          goto _EmitX86M;
        }

        if (o0->getSize() == 10 && info.hasInstFlag(kX86InstFlagMem10)) {
          opCode = extendedInfo.getSecondaryOpCode();
          opReg  = x86ExtractO(opCode);
          goto _EmitX86M;
        }
      }

      if (encoded == ENC_OPS(Reg, None, None)) {
        if (code == kX86InstIdFld) {
          opCode = 0xD9C0 + x86OpReg(o0);
          goto _EmitFpuOp;
        }

        if (code == kX86InstIdFst) {
          opCode = 0xDDD0 + x86OpReg(o0);
          goto _EmitFpuOp;
        }

        if (code == kX86InstIdFstp) {
          opCode = 0xDDD8 + x86OpReg(o0);
          goto _EmitFpuOp;
        }
      }
      break;


    case kX86InstEncodingIdFpuM:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);

        if (o0->getSize() == 2 && info.hasInstFlag(kX86InstFlagMem2)) {
          opCode += 4;
          goto _EmitX86M;
        }

        if (o0->getSize() == 4 && info.hasInstFlag(kX86InstFlagMem4)) {
          goto _EmitX86M;
        }

        if (o0->getSize() == 8 && info.hasInstFlag(kX86InstFlagMem8)) {
          opCode = extendedInfo.getSecondaryOpCode();
          opReg  = x86ExtractO(opCode);
          goto _EmitX86M;
        }
      }
      break;

    case kX86InstEncodingIdFpuRDef:
      if (encoded == ENC_OPS(None, None, None)) {
        opCode += 1;
        goto _EmitFpuOp;
      }
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdFpuR:
      if (encoded == ENC_OPS(Reg, None, None)) {
        opCode += x86OpReg(o0);
        goto _EmitFpuOp;
      }
      break;

    case kX86InstEncodingIdFpuStsw:
      if (encoded == ENC_OPS(Reg, None, None)) {
        if (x86OpReg(o0) != 0)
          goto _IllegalInst;

        opCode = extendedInfo.getSecondaryOpCode();
        goto _EmitFpuOp;
      }

      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Ext]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdExtCrc:
      ADD_66H_P_BY_SIZE(o0->getSize());
      ADD_REX_W_BY_SIZE(o0->getSize());

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        if (!Utils::inInterval<uint32_t>(static_cast<const X86Reg*>(o0)->getRegType(), kX86RegTypeGpd, kX86RegTypeGpq))
          goto _IllegalInst;

        opCode += o0->getSize() != 1;
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        if (!Utils::inInterval<uint32_t>(static_cast<const X86Reg*>(o0)->getRegType(), kX86RegTypeGpd, kX86RegTypeGpq))
          goto _IllegalInst;

        opCode += o0->getSize() != 1;
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtExtrW:
      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o1)));

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        // Secondary opcode of 'pextrw' instruction (SSE4.1).
        opCode = extendedInfo.getSecondaryOpCode();
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o1)));

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtExtract:
      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o1)));

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o1)));

        imVal = static_cast<const Imm*>(o2)->getInt64();
        imLen = 1;

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtFence:
      if (Arch == kArchX64 && (opCode & kX86InstOpCode_W_Mask)) {
        EMIT_BYTE(kX86ByteRex | kX86ByteRexW);
      }

      EMIT_BYTE(0x0F);
      EMIT_BYTE(opCode);
      EMIT_BYTE(0xC0 | (opReg << 3));
      goto _EmitDone;

    case kX86InstEncodingIdExtMov:
    case kX86InstEncodingIdExtMovNoRexW:
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
        ADD_REX_W(static_cast<const X86Reg*>(o0)->isGpq() && (info.getEncodingId() != kX86InstEncodingIdExtMovNoRexW));
        ADD_REX_W(static_cast<const X86Reg*>(o1)->isGpq() && (info.getEncodingId() != kX86InstEncodingIdExtMovNoRexW));

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      // Gp|Mm|Xmm <- Mem
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_REX_W(static_cast<const X86Reg*>(o0)->isGpq() && (info.getEncodingId() != kX86InstEncodingIdExtMovNoRexW));

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      // The following instruction uses opCode[1].
      opCode = extendedInfo.getSecondaryOpCode();

      // X86Mem <- Gp|Mm|Xmm
      if (encoded == ENC_OPS(Mem, Reg, None)) {
        ADD_REX_W(static_cast<const X86Reg*>(o1)->isGpq() && (info.getEncodingId() != kX86InstEncodingIdExtMovNoRexW));

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtMovBe:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        if (o0->getSize() == 1)
          goto _IllegalInst;

        ADD_66H_P_BY_SIZE(o0->getSize());
        ADD_REX_W_BY_SIZE(o0->getSize());

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        if (o1->getSize() == 1)
          goto _IllegalInst;

        ADD_66H_P_BY_SIZE(o1->getSize());
        ADD_REX_W_BY_SIZE(o1->getSize());

        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtMovD:
_EmitMmMovD:
      opReg = x86OpReg(o0);
      ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)));

      // Mm/Xmm <- Gp
      if (encoded == ENC_OPS(Reg, Reg, None) && static_cast<const X86Reg*>(o1)->isGp()) {
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      // Mm/Xmm <- Mem
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg = x86OpReg(o1);
      ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o1)));

      // Gp <- Mm/Xmm
      if (encoded == ENC_OPS(Reg, Reg, None) && static_cast<const X86Reg*>(o0)->isGp()) {
        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }

      // X86Mem <- Mm/Xmm
      if (encoded == ENC_OPS(Mem, Reg, None)) {
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtMovQ:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);

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
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);

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
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);

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
        // Movq in other case is simply a MOVD instruction promoted to 64-bit.
        opCode |= kX86InstOpCode_W;
        goto _EmitMmMovD;
      }
      break;

    case kX86InstEncodingIdExtPrefetch:
      if (encoded == ENC_OPS(Mem, Imm, None)) {
        opReg = static_cast<const Imm*>(o1)->getUInt32() & 0x3;
        rmMem = x86OpMem(o0);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtRm_PQ:
      ADD_66H_P(o0->isRegType(kX86RegTypeXmm) || o1->isRegType(kX86RegTypeXmm));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdExtRm_Q:
      ADD_REX_W(o0->isRegType(kX86RegTypeGpq) || o1->isRegType(kX86RegTypeGpq) || (o1->isMem() && o1->getSize() == 8));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdExtRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtRm_P:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)) | x86IsXmm(static_cast<const X86Reg*>(o1)));

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)));

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtRmRi:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg  = x86ExtractO(opCode);

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }
      break;

    case kX86InstEncodingIdExtRmRi_P:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)) | x86IsXmm(static_cast<const X86Reg*>(o1)));

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)));

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();
      opReg  = x86ExtractO(opCode);

      if (encoded == ENC_OPS(Reg, Imm, None)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)));

        imVal = static_cast<const Imm*>(o1)->getInt64();
        imLen = 1;

        rmReg = x86OpReg(o0);
        goto _EmitX86R;
      }
      break;

    case kX86InstEncodingIdExtRmi:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    case kX86InstEncodingIdExtRmi_P:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)) | x86IsXmm(static_cast<const X86Reg*>(o1)));

        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        ADD_66H_P(x86IsXmm(static_cast<const X86Reg*>(o0)));

        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Group - Extrq / Insertq (SSE4a)]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdExtExtrq:
      opReg = x86OpReg(o0);
      rmReg = x86OpReg(o1);

      if (encoded == ENC_OPS(Reg, Reg, None))
        goto _EmitX86R;

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Reg, Imm, Imm)) {
        imVal = (static_cast<const Imm*>(o1)->getUInt32()     ) +
                (static_cast<const Imm*>(o2)->getUInt32() << 8) ;
        imLen = 2;

        rmReg = x86ExtractO(opCode);
        goto _EmitX86R;
      }
      break;

    case kX86InstEncodingIdExtInsertq:
      opReg = x86OpReg(o0);
      rmReg = x86OpReg(o1);

      if (encoded == ENC_OPS(Reg, Reg, None))
        goto _EmitX86R;

      // The following instruction uses the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Reg, Reg, Imm) && o3->isImm()) {
        imVal = (static_cast<const Imm*>(o2)->getUInt32()     ) +
                (static_cast<const Imm*>(o3)->getUInt32() << 8) ;
        imLen = 2;
        goto _EmitX86R;
      }
      break;

    // ------------------------------------------------------------------------
    // [Group - 3dNow]
    // ------------------------------------------------------------------------

    case kX86InstEncodingId3dNow:
      // Every 3dNow instruction starts with 0x0F0F and the actual opcode is
      // stored as 8-bit immediate.
      imVal = opCode & 0xFF;
      imLen = 1;

      opCode = kX86InstOpCode_MM_0F | 0x0F;
      opReg = x86OpReg(o0);

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        rmReg = x86OpReg(o1);
        goto _EmitX86R;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        rmMem = x86OpMem(o1);
        goto _EmitX86M;
      }
      break;

    // ------------------------------------------------------------------------
    // [Avx]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdAvxOp:
      goto _EmitAvxOp;

    case kX86InstEncodingIdAvxM:
      if (encoded == ENC_OPS(Mem, None, None)) {
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxMr_P:
      ADD_VEX_L(x86IsYmm(o0) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxMr:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxMri_P:
      ADD_VEX_L(x86IsYmm(o0) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxMri:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, Imm)) {
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRm_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRmi:
      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvm_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvm:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
_EmitAvxRvm:
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmr_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvmr:
      if (!o3->isReg())
        goto _IllegalInst;

      imVal = x86OpReg(o3) << 4;
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvmi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRmv:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRmvi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxMovDQ:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        if (static_cast<const X86Reg*>(o0)->isGp()) {
          opCode = extendedInfo.getSecondaryOpCode();
          opReg = x86OpReg(o1);
          rmReg = x86OpReg(o0);
          goto _EmitAvxR;
        }

        if (static_cast<const X86Reg*>(o1)->isGp()) {
          opReg = x86OpReg(o0);
          rmReg = x86OpReg(o1);
          goto _EmitAvxR;
        }
      }

      goto _AvxRmMr_AfterRegRegCheck;

    case kX86InstEncodingIdAvxRmMr_P:
      ADD_VEX_L(x86IsYmm(o0) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRmMr:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

_AvxRmMr_AfterRegRegCheck:
      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmRmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvmRmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }

      // The following instructions use the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmMr:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o1);
        rmReg = x86OpReg(o0);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmMvr_P:
      ADD_VEX_L(x86IsYmm(o0) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvmMvr:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();

      if (encoded == ENC_OPS(Mem, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o2), x86OpReg(o1));
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvmVmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvmVmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitAvxM;
      }

      // The following instruction uses the secondary opcode.
      opCode &= kX86InstOpCode_L_Mask;
      opCode |= extendedInfo.getSecondaryOpCode();
      opReg = x86ExtractO(opCode);

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxVm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxVmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxVmi:
      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmReg = x86OpReg(o1);
        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86RegAndVvvv(opReg, x86OpReg(o0));
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxRvrmRvmr_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdAvxRvrmRvmr:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);

        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = x86OpReg(o2) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o3);

        ADD_VEX_W(true);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);

        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxMovSsSd:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        goto _EmitAvxRvm;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Mem, Reg, None)) {
        opCode = extendedInfo.getSecondaryOpCode();
        opReg = x86OpReg(o1);
        rmMem = x86OpMem(o0);
        goto _EmitAvxM;
      }
      break;

    case kX86InstEncodingIdAvxGatherEx:
      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);

        uint32_t vSib = rmMem->getVSib();
        if (vSib == kX86MemVSibGpz)
          goto _IllegalInst;

        ADD_VEX_L(vSib == kX86MemVSibYmm);
        goto _EmitAvxV;
      }
      break;

    case kX86InstEncodingIdAvxGather:
      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);

        uint32_t vSib = rmMem->getVSib();
        if (vSib == kX86MemVSibGpz)
          goto _IllegalInst;

        ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o2)));
        goto _EmitAvxV;
      }
      break;

    // ------------------------------------------------------------------------
    // [FMA4]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdFma4_P:
      // It's fine to just check the first operand, second is just for sanity.
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdFma4:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);

        goto _EmitAvxR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = x86OpReg(o2) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o3);

        ADD_VEX_W(true);
        goto _EmitAvxM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);

        goto _EmitAvxM;
      }
      break;

    // ------------------------------------------------------------------------
    // [XOP]
    // ------------------------------------------------------------------------

    case kX86InstEncodingIdXopRm_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(o1));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdXopRm:
      if (encoded == ENC_OPS(Reg, Reg, None)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, None)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitXopM;
      }
      break;

    case kX86InstEncodingIdXopRvmRmv:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmReg = x86OpReg(o1);

        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);

        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      break;

    case kX86InstEncodingIdXopRvmRmi:
      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmReg = x86OpReg(o1);
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o2));
        rmMem = x86OpMem(o1);

        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      // The following instructions use the secondary opcode.
      opCode = extendedInfo.getSecondaryOpCode();

      imVal = static_cast<const Imm*>(o2)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Imm)) {
        opReg = x86OpReg(o0);
        rmReg = x86OpReg(o1);
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Mem, Imm)) {
        opReg = x86OpReg(o0);
        rmMem = x86OpMem(o1);
        goto _EmitXopM;
      }
      break;

    case kX86InstEncodingIdXopRvmr_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdXopRvmr:
      if (!o3->isReg())
        goto _IllegalInst;

      imVal = x86OpReg(o3) << 4;
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitXopM;
      }
      break;

    case kX86InstEncodingIdXopRvmi_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdXopRvmi:
      if (!o3->isImm())
        goto _IllegalInst;

      imVal = static_cast<const Imm*>(o3)->getInt64();
      imLen = 1;

      if (encoded == ENC_OPS(Reg, Reg, Reg)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);
        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem)) {
        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);
        goto _EmitXopM;
      }
      break;

    case kX86InstEncodingIdXopRvrmRvmr_P:
      ADD_VEX_L(x86IsYmm(static_cast<const X86Reg*>(o0)) | x86IsYmm(static_cast<const X86Reg*>(o1)));
      ASMJIT_FALLTHROUGH;

    case kX86InstEncodingIdXopRvrmRvmr:
      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmReg = x86OpReg(o2);

        goto _EmitXopR;
      }

      if (encoded == ENC_OPS(Reg, Reg, Reg) && o3->isMem()) {
        imVal = x86OpReg(o2) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o3);

        ADD_VEX_W(true);
        goto _EmitXopM;
      }

      if (encoded == ENC_OPS(Reg, Reg, Mem) && o3->isReg()) {
        imVal = x86OpReg(o3) << 4;
        imLen = 1;

        opReg = x86RegAndVvvv(x86OpReg(o0), x86OpReg(o1));
        rmMem = x86OpMem(o2);

        goto _EmitXopM;
      }
      break;
  }

  // --------------------------------------------------------------------------
  // [Illegal]
  // --------------------------------------------------------------------------

_IllegalInst:
  self->setLastError(kErrorIllegalInst);
#if defined(ASMJIT_DEBUG)
  assertIllegal = true;
#endif // ASMJIT_DEBUG
  goto _EmitDone;

_IllegalAddr:
  self->setLastError(kErrorIllegalAddresing);
#if defined(ASMJIT_DEBUG)
  assertIllegal = true;
#endif // ASMJIT_DEBUG
  goto _EmitDone;

_IllegalDisp:
  self->setLastError(kErrorIllegalDisplacement);
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
  if (Arch == kArchX64) {
    uint32_t rex = x86RexFromOpCodeAndOptions(opCode, options);

    if (rex & ~static_cast<uint32_t>(_kX86InstOptionNoRex)) {
      rex |= kX86ByteRex;
      EMIT_BYTE(rex);

      if (x86RexIsInvalid(rex))
        goto _IllegalInst;
    }
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_BYTE(opCode);

  if (imLen != 0)
    goto _EmitImm;
  else
    goto _EmitDone;

_EmitX86OpWithOpReg:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64) {
    uint32_t rex = x86RexFromOpCodeAndOptions(opCode, options);

    rex += (opReg >> 3); // Rex.B (0x01).

    if (rex & ~static_cast<uint32_t>(_kX86InstOptionNoRex)) {
      rex |= kX86ByteRex;
      opReg &= 0x07;
      EMIT_BYTE(rex);

      if (x86RexIsInvalid(rex))
        goto _IllegalInst;
    }
  }

  // Instruction opcodes.
  opCode += opReg;
  EMIT_MM(opCode);
  EMIT_BYTE(opCode);

  if (imLen != 0)
    goto _EmitImm;
  else
    goto _EmitDone;

_EmitX86R:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Rex prefix (64-bit only).
  if (Arch == kArchX64) {
    uint32_t rex = x86RexFromOpCodeAndOptions(opCode, options);

    rex += static_cast<uint32_t>(opReg & 0x08) >> 1; // Rex.R (0x04).
    rex += static_cast<uint32_t>(rmReg) >> 3;        // Rex.B (0x01).

    if (rex & ~static_cast<uint32_t>(_kX86InstOptionNoRex)) {
      rex |= kX86ByteRex;
      opReg &= 0x07;
      rmReg &= 0x07;
      EMIT_BYTE(rex);

      if (x86RexIsInvalid(rex))
        goto _IllegalInst;
    }
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_BYTE(opCode);

  // ModR.
  EMIT_BYTE(x86EncodeMod(3, opReg, static_cast<uint32_t>(rmReg)));

  if (imLen != 0)
    goto _EmitImm;
  else
    goto _EmitDone;

_EmitX86M:
  ASMJIT_ASSERT(rmMem != nullptr);
  ASMJIT_ASSERT(rmMem->getOp() == Operand::kTypeMem);

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
    uint32_t rex = x86RexFromOpCodeAndOptions(opCode, options);

    rex += static_cast<uint32_t>(opReg      & 8) >> 1; // Rex.R (0x04).
    rex += static_cast<uint32_t>(mIndex - 8 < 8) << 1; // Rex.X (0x02).
    rex += static_cast<uint32_t>(mBase  - 8 < 8);      // Rex.B (0x01).

    if (rex & ~static_cast<uint32_t>(_kX86InstOptionNoRex)) {
      rex |= kX86ByteRex;
      opReg &= 0x07;
      EMIT_BYTE(rex);

      if (x86RexIsInvalid(rex))
        goto _IllegalInst;
    }

    mBase &= 0x07;
  }

  // Instruction opcodes.
  EMIT_MM(opCode);
  EMIT_BYTE(opCode);
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
        else if (Utils::isInt8(dispOffset)) {
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
      else if (Utils::isInt8(dispOffset)) {
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
      mIndex &= 0x07;
      ASMJIT_ASSERT(mIndex != kX86RegIndexSp);

      if (mBase != kX86RegIndexBp && dispOffset == 0) {
        // [Base + Index * Scale].
        EMIT_BYTE(x86EncodeMod(0, opReg, 4));
        EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
      }
      else if (Utils::isInt8(dispOffset)) {
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

    if (rmMem->getMemType() == kMemTypeAbsolute) {
      // [Disp32].
      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
    else if (rmMem->getMemType() == kMemTypeLabel) {
      // Relative->Absolute [x86 mode].
      label = self->getLabelData(rmMem->_vmem.base);
      relocId = self->_relocations.getLength();

      RelocData rd;
      rd.type = kRelocRelToAbs;
      rd.size = 4;
      rd.from = static_cast<Ptr>((uintptr_t)(cursor - self->_buffer));
      rd.data = static_cast<SignedPtr>(dispOffset);

      if (self->_relocations.append(rd) != kErrorOk)
        return self->setLastError(kErrorNoHeapMemory);

      if (label->offset != -1) {
        // Bound label.
        self->_relocations[relocId].data += static_cast<SignedPtr>(label->offset);
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
      // RIP->Absolute [x86 mode].
      relocId = self->_relocations.getLength();

      RelocData rd;
      rd.type = kRelocRelToAbs;
      rd.size = 4;
      rd.from = static_cast<Ptr>((uintptr_t)(cursor - self->_buffer));
      rd.data = rd.from + static_cast<SignedPtr>(dispOffset);

      if (self->_relocations.append(rd) != kErrorOk)
        return self->setLastError(kErrorNoHeapMemory);

      EMIT_DWORD(0);
    }
  }
  else /* if (Arch === kArchX64) */ {
    if (rmMem->getMemType() == kMemTypeAbsolute) {
      EMIT_BYTE(x86EncodeMod(0, opReg, 4));
      if (mIndex >= kInvalidReg) {
        // [Disp32].
        EMIT_BYTE(x86EncodeSib(0, 4, 5));
      }
      else {
        // [Disp32 + Index * Scale].
        mIndex &= 0x07;
        ASMJIT_ASSERT(mIndex != kX86RegIndexSp);

        uint32_t shift = rmMem->getShift();
        EMIT_BYTE(x86EncodeSib(shift, mIndex, 5));
      }
      EMIT_DWORD(static_cast<int32_t>(dispOffset));
    }
    else if (rmMem->getMemType() == kMemTypeLabel) {
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
      // [RIP + Disp32].

      // Indexing is invalid.
      if (mIndex < kInvalidReg)
        goto _IllegalDisp;

      EMIT_BYTE(x86EncodeMod(0, opReg, 5));
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
      ASMJIT_NOT_REACHED();
  }
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Fpu]
  // --------------------------------------------------------------------------

_EmitFpuOp:
  // Mandatory instruction prefix.
  EMIT_PP(opCode);

  // Instruction opcodes.
  EMIT_BYTE(opCode >> 8);
  EMIT_BYTE(opCode);
  goto _EmitDone;

  // --------------------------------------------------------------------------
  // [Emit - Avx]
  // --------------------------------------------------------------------------

#define EMIT_AVX_M \
  ASMJIT_ASSERT(rmMem != nullptr); \
  ASMJIT_ASSERT(rmMem->getOp() == Operand::kTypeMem); \
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
    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_W_Shift - 7)) & 0x80; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift   )) & 0x03; \
    vex_XvvvvLpp += (opReg  >> (kVexVVVVShift          - 3)); \
    \
    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x0F; \
    vex_rxbmmmmm |= static_cast<uint32_t>(mBase  - 8 < 8) << 5; \
    vex_rxbmmmmm |= static_cast<uint32_t>(mIndex - 8 < 8) << 6; \
    \
    if ((vex_rxbmmmmm != 0x01) || (vex_XvvvvLpp >= 0x80) || ((options & kX86InstOptionVex3) != 0)) { \
      vex_rxbmmmmm |= static_cast<uint32_t>(opReg << 4) & 0x80; \
      vex_rxbmmmmm ^= 0xE0; \
      vex_XvvvvLpp ^= 0x78; \
      \
      EMIT_BYTE(kX86ByteVex3); \
      EMIT_BYTE(vex_rxbmmmmm); \
      EMIT_BYTE(vex_XvvvvLpp); \
      EMIT_BYTE(opCode); \
    } \
    else { \
      vex_XvvvvLpp |= static_cast<uint32_t>(opReg << 4) & 0x80; \
      vex_XvvvvLpp ^= 0xF8; \
      \
      EMIT_BYTE(kX86ByteVex2); \
      EMIT_BYTE(vex_XvvvvLpp); \
      EMIT_BYTE(opCode); \
    } \
  } \
  \
  mBase &= 0x07; \
  opReg &= 0x07;

_EmitAvxOp:
  {
    uint32_t vex_XvvvvLpp;

    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    vex_XvvvvLpp |= (opCode >> (kX86InstOpCode_PP_Shift));
    vex_XvvvvLpp |= 0xF8;

    // Encode 3-byte VEX prefix only if specified in options.
    if ((options & kX86InstOptionVex3) != 0) {
      uint32_t vex_rxbmmmmm = (opCode >> kX86InstOpCode_MM_Shift) | 0xE0;

      EMIT_BYTE(kX86ByteVex3);
      EMIT_BYTE(vex_rxbmmmmm);
      EMIT_BYTE(vex_XvvvvLpp);
      EMIT_BYTE(opCode);
    }
    else {
      EMIT_BYTE(kX86ByteVex2);
      EMIT_BYTE(vex_XvvvvLpp);
      EMIT_BYTE(opCode);
    }
  }
  goto _EmitDone;

_EmitAvxR:
  {
    uint32_t vex_XvvvvLpp;
    uint32_t vex_rxbmmmmm;

    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_W_Shift - 7)) & 0x80;
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift   )) & 0x03;
    vex_XvvvvLpp += (opReg  >> (kVexVVVVShift          - 3));

    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x0F;
    vex_rxbmmmmm |= (rmReg << 2) & 0x20;

    if (vex_rxbmmmmm != 0x01 || vex_XvvvvLpp >= 0x80 || (options & kX86InstOptionVex3) != 0) {
      vex_rxbmmmmm |= static_cast<uint32_t>(opReg & 0x08) << 4;
      vex_rxbmmmmm ^= 0xE0;
      vex_XvvvvLpp ^= 0x78;

      EMIT_BYTE(kX86ByteVex3);
      EMIT_BYTE(vex_rxbmmmmm);
      EMIT_BYTE(vex_XvvvvLpp);
      EMIT_BYTE(opCode);

      rmReg &= 0x07;
    }
    else {
      vex_XvvvvLpp += static_cast<uint32_t>(opReg & 0x08) << 4;
      vex_XvvvvLpp ^= 0xF8;

      EMIT_BYTE(kX86ByteVex2);
      EMIT_BYTE(vex_XvvvvLpp);
      EMIT_BYTE(opCode);
    }
  }

  EMIT_BYTE(x86EncodeMod(3, opReg & 0x07, static_cast<uint32_t>(rmReg)));

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
    mIndex &= 0x07;

  dispOffset = rmMem->getDisplacement();
  if (rmMem->isBaseIndexType()) {
    uint32_t shift = rmMem->getShift();

    if (mBase != kX86RegIndexBp && dispOffset == 0) {
      // [Base + Index * Scale].
      EMIT_BYTE(x86EncodeMod(0, opReg, 4));
      EMIT_BYTE(x86EncodeSib(shift, mIndex, mBase));
    }
    else if (Utils::isInt8(dispOffset)) {
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
      relocId = self->_relocations.getLength();

      {
        RelocData rd;
        rd.type = kRelocRelToAbs;
        rd.size = 4;
        rd.from = static_cast<Ptr>((uintptr_t)(cursor - self->_buffer));
        rd.data = static_cast<SignedPtr>(dispOffset);

        if (self->_relocations.append(rd) != kErrorOk)
          return self->setLastError(kErrorNoHeapMemory);
      }

      if (label->offset != -1) {
        // Bound label.
        self->_relocations[relocId].data += static_cast<SignedPtr>(label->offset);
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
  ASMJIT_ASSERT(rmMem != nullptr); \
  ASMJIT_ASSERT(rmMem->getOp() == Operand::kTypeMem); \
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
    vex_XvvvvLpp  = (opCode >> (kX86InstOpCode_W_Shift - 7)) & 0x80; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04; \
    vex_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift   )) & 0x03; \
    vex_XvvvvLpp += (opReg  >> (kVexVVVVShift          - 3)); \
    \
    vex_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x0F; \
    vex_rxbmmmmm += static_cast<uint32_t>(mBase  - 8 < 8) << 5; \
    vex_rxbmmmmm += static_cast<uint32_t>(mIndex - 8 < 8) << 6; \
    \
    vex_rxbmmmmm |= static_cast<uint32_t>(opReg << 4) & 0x80; \
    vex_rxbmmmmm ^= 0xE0; \
    vex_XvvvvLpp ^= 0x78; \
    \
    EMIT_BYTE(kX86ByteXop3); \
    EMIT_BYTE(vex_rxbmmmmm); \
    EMIT_BYTE(vex_XvvvvLpp); \
    EMIT_BYTE(opCode); \
  } \
  \
  mBase &= 0x07; \
  opReg &= 0x07;

_EmitXopR:
  {
    uint32_t xop_XvvvvLpp;
    uint32_t xop_rxbmmmmm;

    xop_XvvvvLpp  = (opCode >> (kX86InstOpCode_W_Shift - 7)) & 0x80;
    xop_XvvvvLpp += (opCode >> (kX86InstOpCode_L_Shift - 2)) & 0x04;
    xop_XvvvvLpp += (opCode >> (kX86InstOpCode_PP_Shift   )) & 0x03;
    xop_XvvvvLpp += (opReg  >> (kVexVVVVShift          - 3));

    xop_rxbmmmmm  = (opCode >> kX86InstOpCode_MM_Shift) & 0x0F;
    xop_rxbmmmmm |= (rmReg << 2) & 0x20;

    xop_rxbmmmmm |= static_cast<uint32_t>(opReg & 0x08) << 4;
    xop_rxbmmmmm ^= 0xE0;
    xop_XvvvvLpp ^= 0x78;

    EMIT_BYTE(kX86ByteXop3);
    EMIT_BYTE(xop_rxbmmmmm);
    EMIT_BYTE(xop_XvvvvLpp);
    EMIT_BYTE(opCode);

    rmReg &= 0x07;
  }

  EMIT_BYTE(x86EncodeMod(3, opReg & 0x07, static_cast<uint32_t>(rmReg)));

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
_EmitJmpOrCallAbs:
  {
    RelocData rd;
    rd.type = kRelocAbsToRel;
    rd.size = 4;
    rd.from = (intptr_t)(cursor - self->_buffer) + 1;
    rd.data = static_cast<SignedPtr>(imVal);

    uint32_t trampolineSize = 0;

    if (Arch == kArchX64) {
      Ptr baseAddress = self->getRuntime()->getBaseAddress();

      // If the base address of the output is known, it's possible to determine
      // the need for a trampoline here. This saves possible REX prefix in
      // 64-bit mode and prevents reserving space needed for an absolute address.
      if (baseAddress == kNoBaseAddress || !x64IsRelative(rd.data, baseAddress + rd.from + 4)) {
        // Emit REX prefix so the instruction can be patched later on. The REX
        // prefix does nothing if not patched after, but allows to patch the
        // instruction in case where the trampoline is needed.
        rd.type = kRelocTrampoline;
        rd.from++;

        EMIT_BYTE(kX86ByteRex);
        trampolineSize = 8;
      }
    }

    // Both `jmp` and `call` instructions have a single-byte opcode and are
    // followed by a 32-bit displacement.
    EMIT_BYTE(opCode);
    EMIT_DWORD(0);

    if (self->_relocations.append(rd) != kErrorOk)
      return self->setLastError(kErrorNoHeapMemory);

    // Reserve space for a possible trampoline.
    self->_trampolinesSize += trampolineSize;
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

    if (imLen != 0)
      goto _EmitImm;
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
    StringBuilderTmp<512> sb;
    uint32_t loggerOptions = 0;

    if (self->_logger) {
      sb.appendString(self->_logger->getIndentation());
      loggerOptions = self->_logger->getOptions();
    }

    X86Assembler_dumpInstruction(sb, Arch, code, options, o0, o1, o2, o3, loggerOptions);

    if ((loggerOptions & Logger::kOptionBinaryForm) != 0)
      LogUtil::formatLine(sb, self->_cursor, (intptr_t)(cursor - self->_cursor), dispSize, imLen, self->_comment);
    else
      LogUtil::formatLine(sb, nullptr, kInvalidIndex, 0, 0, self->_comment);

# if defined(ASMJIT_DEBUG)
    if (self->_logger)
# endif // ASMJIT_DEBUG
      self->_logger->logString(Logger::kStyleDefault, sb.getData(), sb.getLength());

# if defined(ASMJIT_DEBUG)
    // This shouldn't happen.
    if (assertIllegal)
      DebugUtils::assertionFailed(__FILE__, __LINE__, sb.getData());
# endif // ASMJIT_DEBUG
  }
#else
# if defined(ASMJIT_DEBUG)
  ASMJIT_ASSERT(!assertIllegal);
# endif // ASMJIT_DEBUG
#endif // !ASMJIT_DISABLE_LOGGER

  self->_comment = nullptr;
  self->setCursor(cursor);

  return kErrorOk;
}

Error X86Assembler::_emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) {
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_BUILD_X64)
  ASMJIT_ASSERT(_arch == kArchX86);
  return X86Assembler_emit<kArchX86>(this, code, &o0, &o1, &o2, &o3);
#elif !defined(ASMJIT_BUILD_X86) && defined(ASMJIT_BUILD_X64)
  ASMJIT_ASSERT(_arch == kArchX64);
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
