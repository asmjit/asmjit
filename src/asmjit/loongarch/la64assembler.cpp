// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_LOONGARCH64)

#include "../core/codewriter_p.h"
#include "../core/cpuinfo.h"
#include "../core/emitterutils_p.h"
#include "../core/formatter.h"
#include "../core/logger.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../loongarch/la64assembler.h"
#include "../loongarch/la64emithelper_p.h"
#include "../loongarch/la64instdb_p.h"
#include "../loongarch/laformatter_p.h"
#include "../loongarch/lautils.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

// a64::Assembler - Utils
// ======================

static ASMJIT_FORCE_INLINE constexpr uint32_t diff(RegType a,
                                                   RegType b) noexcept {
  return uint32_t(a) - uint32_t(b);
}
static ASMJIT_FORCE_INLINE constexpr uint32_t diff(
    VecElementType elementType, VecElementType baseType) noexcept {
  return uint32_t(elementType) - uint32_t(baseType);
}

// a64::Assembler - Bits
// =====================

template <typename T>
static inline constexpr uint32_t B(const T& index) noexcept {
  return uint32_t(1u) << uint32_t(index);
}

static constexpr uint32_t kSP = Gp::kIdSp;
static constexpr uint32_t kZR = Gp::kIdZr;
static constexpr uint32_t kWX = InstDB::kWX;

// a64::Assembler - ShiftOpToLdStOptMap
// ====================================

// asmjit::a64::Assembler - SizeOp
// ===============================

//! Struct that contains Size (2 bits), Q flag, and S (scalar) flag. These
//! values are used to encode Q, Size, and Scalar fields in an opcode.
struct SizeOp {
  enum : uint8_t {
    k128BitShift = 0,
    kScalarShift = 1,
    kSizeShift = 2,

    kQ = uint8_t(1u << k128BitShift),
    kS = uint8_t(1u << kScalarShift),

    k00 = uint8_t(0 << kSizeShift),
    k01 = uint8_t(1 << kSizeShift),
    k10 = uint8_t(2 << kSizeShift),
    k11 = uint8_t(3 << kSizeShift),

    k00Q = k00 | kQ,
    k01Q = k01 | kQ,
    k10Q = k10 | kQ,
    k11Q = k11 | kQ,

    k00S = k00 | kS,
    k01S = k01 | kS,
    k10S = k10 | kS,
    k11S = k11 | kS,

    kInvalid = 0xFFu,

    // Masks used by SizeOpMap.
    kSzQ = (0x3u << kSizeShift) | kQ,
    kSzS = (0x3u << kSizeShift) | kS,
    kSzQS = (0x3u << kSizeShift) | kQ | kS
  };

  uint8_t value;

  inline bool isValid() const noexcept { return value != kInvalid; }
  inline void makeInvalid() noexcept { value = kInvalid; }

  inline uint32_t q() const noexcept { return (value >> k128BitShift) & 0x1u; }
  inline uint32_t qs() const noexcept {
    return ((value >> k128BitShift) | (value >> kScalarShift)) & 0x1u;
  }
  inline uint32_t scalar() const noexcept {
    return (value >> kScalarShift) & 0x1u;
  }
  inline uint32_t size() const noexcept { return (value >> kSizeShift) & 0x3u; }

  inline void decrementSize() noexcept {
    ASMJIT_ASSERT(size() > 0);
    value = uint8_t(value - (1u << kSizeShift));
  }
};

struct SizeOpTable {
  enum TableId : uint8_t { kTableBin = 0, kTableAny, kCount };

  // 40 elements for each combination.
  SizeOp array[(uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB) + 1) *
               8];
};

#define VALUE_BIN(x)                                                           \
  {                                                                            \
    x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB)) << 3) | \
          uint32_t(VecElementType::kNone))                                     \
        ? SizeOp::k00                                                          \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kNone))                                   \
        ? SizeOp::k00Q                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kB))                                      \
        ? SizeOp::k00                                                          \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kB))                                      \
        ? SizeOp::k00Q                                                         \
        : SizeOp::kInvalid                                                     \
  }

#define VALUE_ANY(x)                                                           \
  {                                                                            \
    x == (((uint32_t(RegType::kLA_VecB) - uint32_t(RegType::kLA_VecB)) << 3) | \
          uint32_t(VecElementType::kNone))                                     \
        ? SizeOp::k00S                                                         \
    : x == (((uint32_t(RegType::kLA_VecH) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kNone))                                   \
        ? SizeOp::k01S                                                         \
    : x == (((uint32_t(RegType::kLA_VecS) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kNone))                                   \
        ? SizeOp::k10S                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kNone))                                   \
        ? SizeOp::k11S                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kB))                                      \
        ? SizeOp::k00                                                          \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kB))                                      \
        ? SizeOp::k00Q                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kH))                                      \
        ? SizeOp::k01                                                          \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kH))                                      \
        ? SizeOp::k01Q                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kW))                                      \
        ? SizeOp::k10                                                          \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kW))                                      \
        ? SizeOp::k10Q                                                         \
    : x == (((uint32_t(RegType::kLA_VecD) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kD))                                      \
        ? SizeOp::k11S                                                         \
    : x == (((uint32_t(RegType::kLA_VecV) - uint32_t(RegType::kLA_VecB))       \
             << 3) |                                                           \
            uint32_t(VecElementType::kD))                                      \
        ? SizeOp::k11Q                                                         \
        : SizeOp::kInvalid                                                     \
  }

#undef VALUE_ANY
#undef VALUE_BIN

struct SizeOpMap {
  uint8_t tableId;
  uint8_t sizeOpMask;
  uint16_t acceptMask;
};

// a64::Assembler - Immediate Encoding Utilities (Integral)
// ========================================================

using Utils::LogicalImm;

struct HalfWordImm {
  uint32_t hw;
  uint32_t inv;
  uint32_t imm;
};

struct LMHImm {
  uint32_t lm;
  uint32_t h;
  uint32_t maxRmId;
};

// a64::Assembler - Opcode
// =======================

//! Helper class to store and manipulate LA opcode.
struct Opcode {
  uint32_t v;

  enum Bits : uint32_t { kN = (1u << 22), kQ = (1u << 30), kX = (1u << 31) };

  // --------------------------------------------------------------------------
  // [Opcode Builder]
  // --------------------------------------------------------------------------

  inline uint32_t get() const noexcept { return v; }
  inline void reset(uint32_t value) noexcept { v = value; }

  inline bool hasQ() const noexcept { return (v & kQ) != 0; }
  inline bool hasX() const noexcept { return (v & kX) != 0; }

  template <typename T>
  inline Opcode& addImm(T value, uint32_t bitIndex) noexcept {
    return operator|=(uint32_t(value) << bitIndex);
  }

  template <typename T>
  inline Opcode& xorImm(T value, uint32_t bitIndex) noexcept {
    return operator^=(uint32_t(value) << bitIndex);
  }
  inline Opcode& shiftopR(uint32_t bitIndex) noexcept {
    return operator>>=(bitIndex);
  }
  inline Opcode& shiftopL(uint32_t bitIndex) noexcept {
    return operator<<=(bitIndex);
  }

  template <typename T, typename Condition>
  inline Opcode& addIf(T value, const Condition& condition) noexcept {
    return operator|=(condition ? uint32_t(value) : uint32_t(0));
  }

  inline Opcode& addLogicalImm(const LogicalImm& logicalImm) noexcept {
    addImm(logicalImm.n, 22);
    addImm(logicalImm.r, 16);
    addImm(logicalImm.s, 10);
    return *this;
  }

  inline Opcode& addReg(uint32_t id, uint32_t bitIndex) noexcept {
    return operator|=((id & 31u) << bitIndex);
  }
  inline Opcode& addReg(const Operand_& op, uint32_t bitIndex) noexcept {
    return addReg(op.id(), bitIndex);
  }

  inline Opcode& operator=(uint32_t x) noexcept {
    v = x;
    return *this;
  }
  inline Opcode& operator&=(uint32_t x) noexcept {
    v &= x;
    return *this;
  }
  inline Opcode& operator>>=(uint32_t x) noexcept {
    v >>= x;
    return *this;
  }
  inline Opcode& operator<<=(uint32_t x) noexcept {
    v <<= x;
    return *this;
  }
  inline Opcode& operator|=(uint32_t x) noexcept {
    v |= x;
    return *this;
  }
  inline Opcode& operator^=(uint32_t x) noexcept {
    v ^= x;
    return *this;
  }

  inline uint32_t operator&(uint32_t x) const noexcept { return v & x; }
  inline uint32_t operator|(uint32_t x) const noexcept { return v | x; }
  inline uint32_t operator^(uint32_t x) const noexcept { return v ^ x; }
};

// a64::Assembler - Signature Utilities
// ====================================

// TODO:  Deprecate matchSignature.
static inline bool matchSignature(const Operand_& o0, const Operand_& o1,
                                  uint32_t instFlags) noexcept {
  if (!(instFlags & (InstDB::kInstFlagLong | InstDB::kInstFlagNarrow)))
    return o0.signature() == o1.signature();

  // TODO: Something smart to validate this.
  return true;
}

static inline bool matchSignature(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2,
                                  uint32_t instFlags) noexcept {
  return matchSignature(o0, o1, instFlags) && o1.signature() == o2.signature();
}

static inline bool matchSignature(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2, const Operand_& o3,
                                  uint32_t instFlags) noexcept {
  return matchSignature(o0, o1, instFlags) &&
         o1.signature() == o2.signature() && o2.signature() == o3.signature();
}

// Memory must be either:
// 1. Absolute address, which will be converted to relative.
// 2. Relative displacement (Label).
// 3. Base register + either offset or index.
static inline bool laCheckMemBaseIndexRel(const Mem& mem) noexcept {
  // Allowed base types (Nothing, Label, and GpX).
  constexpr uint32_t kBaseMask =
      B(0) | B(RegType::kLabelTag) | B(RegType::kLA_GpX);

  // Allowed index types (Nothing, GpW, and GpX).
  constexpr uint32_t kIndexMask =
      B(0) | B(RegType::kLA_GpW) | B(RegType::kLA_GpX);

  RegType baseType = mem.baseType();
  RegType indexType = mem.indexType();

  if (!Support::bitTest(kBaseMask, baseType)) return false;

  if (baseType > RegType::kLabelTag) {
    // Index allows either GpW or GpX.
    if (!Support::bitTest(kIndexMask, indexType)) return false;

    if (indexType == RegType::kNone)
      return true;
    else
      return !mem.hasOffset();
  } else {
    // No index register allowed if this is a PC relative address (literal).
    return indexType == RegType::kNone;
  }
}

struct EncodeFpOpcodeBits {
  uint32_t sizeMask;
  uint32_t mask[3];
};

// a64::Assembler - Operand Checks
// ===============================

// Checks whether all operands have the same signature.
static inline bool checkSignature(const Operand_& o0,
                                  const Operand_& o1) noexcept {
  return o0.signature() == o1.signature();
}

static inline bool checkSignature(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2) noexcept {
  return o0.signature() == o1.signature() && o1.signature() == o2.signature();
}

static inline bool checkSignature(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2,
                                  const Operand_& o3) noexcept {
  return o0.signature() == o1.signature() && o1.signature() == o2.signature() &&
         o2.signature() == o3.signature();
}

// Checks whether the register is GP register of the allowed types.
//
// Allowed is a 2-bit mask, where the first bits allows GpW and the second bit
// allows GpX. These bits are usually stored within the instruction, but could
// be also hardcoded in the assembler for instructions where GP types are not
// selectable.
static inline bool checkGpType(const Operand_& op, uint32_t allowed) noexcept {
  RegType type = op.as<Reg>().type();
  return Support::bitTest(allowed << uint32_t(RegType::kLA_GpW), type);
}

static inline bool checkGpType(const Operand_& op, uint32_t allowed,
                               uint32_t* x) noexcept {
  // NOTE: We set 'x' to one only when GpW is allowed, otherwise the X is part
  // of the opcode and we cannot set it. This is why this works without
  // requiring additional logic.
  RegType type = op.as<Reg>().type();
  *x = diff(type, RegType::kLA_GpW) & allowed;
  return Support::bitTest(allowed << uint32_t(RegType::kLA_GpW), type);
}

static inline bool checkGpType(const Operand_& o0, const Operand_& o1,
                               uint32_t allowed, uint32_t* x) noexcept {
  return checkGpType(o0, allowed, x) && checkSignature(o0, o1);
}

static inline bool checkGpType(const Operand_& o0, const Operand_& o1,
                               const Operand_& o2, uint32_t allowed,
                               uint32_t* x) noexcept {
  return checkGpType(o0, allowed, x) && checkSignature(o0, o1, o2);
}

static inline bool checkGpId(const Operand_& op, uint32_t hiId = kZR) noexcept {
  uint32_t id = op.as<Reg>().id();
  return id < 31u || id == hiId;
}

static inline bool checkGpId(const Operand_& o0, const Operand_& o1,
                             uint32_t hiId = kZR) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();

  return (id0 < 31u || id0 == hiId) && (id1 < 31u || id1 == hiId);
}

static inline bool checkGpId(const Operand_& o0, const Operand_& o1,
                             const Operand_& o2, uint32_t hiId = kZR) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();
  uint32_t id2 = o2.as<Reg>().id();

  return (id0 < 31u || id0 == hiId) && (id1 < 31u || id1 == hiId) &&
         (id2 < 31u || id2 == hiId);
}

static inline bool checkVecId(const Operand_& o0, const Operand_& o1) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();

  return (id0 | id1) <= 31u;
}

static inline bool checkMemBase(const Mem& mem) noexcept {
  return mem.baseType() == RegType::kLA_GpX && mem.baseId() <= 31;
}

// a64::Assembler - CheckReg
// =========================

#define V(index)                                      \
  (index == uint32_t(RegType::kLA_GpW)    ? Gp::kIdZr \
   : index == uint32_t(RegType::kLA_GpX)  ? Gp::kIdZr \
   : index == uint32_t(RegType::kLA_VecB) ? 31u       \
   : index == uint32_t(RegType::kLA_VecH) ? 31u       \
   : index == uint32_t(RegType::kLA_VecS) ? 31u       \
   : index == uint32_t(RegType::kLA_VecD) ? 31u       \
   : index == uint32_t(RegType::kLA_VecV) ? 31u       \
                                          : 0)
static const Support::Array<uint8_t, 32> commonHiRegIdOfType = {
    {ASMJIT_LOOKUP_TABLE_32(V, 0)}};
#undef V

static inline bool checkValidRegs(const Operand_& o0) noexcept {
  return bool(unsigned(o0.id() < 31) |
              unsigned(o0.id() == commonHiRegIdOfType[o0.as<Reg>().type()]));
}

static inline bool checkValidRegs(const Operand_& o0,
                                  const Operand_& o1) noexcept {
  return bool((unsigned(o0.id() < 31) |
               unsigned(o0.id() == commonHiRegIdOfType[o0.as<Reg>().type()])) &
              (unsigned(o1.id() < 31) |
               unsigned(o1.id() == commonHiRegIdOfType[o1.as<Reg>().type()])));
}

static inline bool checkValidRegs(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2) noexcept {
  return bool((unsigned(o0.id() < 31) |
               unsigned(o0.id() == commonHiRegIdOfType[o0.as<Reg>().type()])) &
              (unsigned(o1.id() < 31) |
               unsigned(o1.id() == commonHiRegIdOfType[o1.as<Reg>().type()])) &
              (unsigned(o2.id() < 31) |
               unsigned(o2.id() == commonHiRegIdOfType[o2.as<Reg>().type()])));
}

static inline bool checkValidRegs(const Operand_& o0, const Operand_& o1,
                                  const Operand_& o2,
                                  const Operand_& o3) noexcept {
  return bool((unsigned(o0.id() < 31) |
               unsigned(o0.id() == commonHiRegIdOfType[o0.as<Reg>().type()])) &
              (unsigned(o1.id() < 31) |
               unsigned(o1.id() == commonHiRegIdOfType[o1.as<Reg>().type()])) &
              (unsigned(o2.id() < 31) |
               unsigned(o2.id() == commonHiRegIdOfType[o2.as<Reg>().type()])) &
              (unsigned(o3.id() < 31) |
               unsigned(o3.id() == commonHiRegIdOfType[o3.as<Reg>().type()])));
}

// a64::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  _archMask = uint64_t(1) << uint32_t(Arch::kLOONGARCH64);
  if (code) code->attach(this);
}

Assembler::~Assembler() noexcept {}

// a64::Assembler - Emit
// =====================

#define ENC_OPS1(OP0) (uint32_t(OperandType::k##OP0))

#define ENC_OPS2(OP0, OP1) \
  (uint32_t(OperandType::k##OP0) + (uint32_t(OperandType::k##OP1) << 3))

#define ENC_OPS3(OP0, OP1, OP2)                                           \
  (uint32_t(OperandType::k##OP0) + (uint32_t(OperandType::k##OP1) << 3) + \
   (uint32_t(OperandType::k##OP2) << 6))

#define ENC_OPS4(OP0, OP1, OP2, OP3)                                      \
  (uint32_t(OperandType::k##OP0) + (uint32_t(OperandType::k##OP1) << 3) + \
   (uint32_t(OperandType::k##OP2) << 6) +                                 \
   (uint32_t(OperandType::k##OP3) << 9))

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1,
                       const Operand_& o2, const Operand_* opExt) {
  // Logging/Validation/Error.
  constexpr InstOptions kRequiresSpecialHandling = InstOptions::kReserved;

  Error err;
  CodeWriter writer(this);

  // Combine all instruction options and also check whether the instruction
  // is valid. All options that require special handling (including invalid
  // instruction) are handled by the next branch.
  InstOptions options =
      InstOptions(instId - 1 >= Inst::_kIdCount - 1) |
      InstOptions((size_t)(_bufferEnd - writer.cursor()) < 4) | instOptions() |
      forcedInstOptions();

  CondCode instCC = BaseInst::extractLACondCode(instId);
  instId = instId & uint32_t(InstIdParts::kRealId);

  if (instId >= Inst::_kIdCount) instId = 0;

  const InstDB::InstInfo* instInfo = &InstDB::_instInfoTable[instId];
  uint32_t encodingIndex = instInfo->_encodingDataIndex;

  Opcode opcode;
  uint32_t isign4;
  uint32_t instFlags;

  const Operand_& o3 = opExt[EmitterUtils::kOp3];
  const Operand_* rmRel = nullptr;

  // These are only used when instruction uses a relative displacement.
  OffsetFormat offsetFormat;  // Offset format.
  uint64_t offsetValue;       // Offset value (if known).

  if (ASMJIT_UNLIKELY(Support::test(options, kRequiresSpecialHandling))) {
    if (ASMJIT_UNLIKELY(!_code))
      return reportError(DebugUtils::errored(kErrorNotInitialized));

    // Unknown instruction.
    if (ASMJIT_UNLIKELY(instId == 0)) goto InvalidInstruction;

    // Condition code can only be used with 'B' instruction.
    if (ASMJIT_UNLIKELY(instCC != CondCode::kAL && instId != Inst::kIdB))
      goto InvalidInstruction;

    // Grow request, happens rarely.
    err = writer.ensureSpace(this, 4);
    if (ASMJIT_UNLIKELY(err)) goto Failed;

#ifndef ASMJIT_NO_VALIDATION
    // Strict validation.
    if (hasDiagnosticOption(DiagnosticOptions::kValidateAssembler)) {
      Operand_ opArray[Globals::kMaxOpCount];
      EmitterUtils::opArrayFromEmitArgs(opArray, o0, o1, o2, opExt);

      err = _funcs.validate(BaseInst(instId, options, _extraReg), opArray,
                            Globals::kMaxOpCount, ValidationFlags::kNone);
      if (ASMJIT_UNLIKELY(err)) goto Failed;
    }
#endif
  }

  // Signature of the first 4 operands.
  isign4 = (uint32_t(o0.opType())) + (uint32_t(o1.opType()) << 3) +
           (uint32_t(o2.opType()) << 6) + (uint32_t(o3.opType()) << 9);
  instFlags = instInfo->flags();

  switch (instInfo->_encoding) {
    // ------------------------------------------------------------------------
    // [Base - Universal]
    // ------------------------------------------------------------------------
    case InstDB::kEncodingBaseOp: {
      const InstDB::EncodingData::BaseOp& opData =
          InstDB::EncodingData::baseOp[encodingIndex];

      if (isign4 == 0) {
        opcode.reset(opData.opcode);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseOpImm: {
      const InstDB::EncodingData::BaseOpImm& opData =
          InstDB::EncodingData::baseOpImm[encodingIndex];

      if (isign4 == ENC_OPS1(Imm)) {
        uint64_t imm = o0.as<Imm>().valueAs<uint64_t>();
        if (imm >= uint64_t(1u << opData.immBits)) goto InvalidImmediate;

        opcode.reset(opData.opcode());
        opcode.addImm(imm, opData.immOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLIC: {
      const InstDB::EncodingData::BaseLIC& opData =
          InstDB::EncodingData::baseLIC[encodingIndex];

      if (isign4 == ENC_OPS1(Imm)) {
        uint64_t imm = o0.as<Imm>().valueAs<uint64_t>();
        if ((imm >> 15) > 0) goto InvalidImmediate;

        opcode.reset(opData.opcode());
        opcode.addImm(imm, opData.immOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRI: {
      const InstDB::EncodingData::BaseLRI& opData =
          InstDB::EncodingData::baseLRI[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (!checkGpType(o0, opData.rType)) goto InvalidInstruction;

        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();

        if ((aImm > 7) && opData.uniform == 1) {
          goto InvalidImmediate;
        }
        if ((opData.uniform == 2 || opData.uniform == 3) &&
            ((aImm >> 14) > 0)) {
          goto InvalidImmediate;
        }
        if (opData.uniform == 4 && ((aImm >> 8) > 0)) {
          goto InvalidImmediate;
        }
        if (((aImm >> 20) > 0) && opData.uniform == 0) {
          aImm &= 0xFFFFF;
        }
        opcode.reset(opData.opcode());
        if (opData.uniform == 0) {
          opcode.shiftopL(15);
        }
        if (opData.uniform == 2 || opData.uniform == 3) {
          opcode.shiftopL(14);
        }
        if (opData.uniform == 4) {
          opcode.shiftopL(8);
        }
        opcode.addImm(aImm, opData.aImmOffset);
        if (opData.uniform == 3) {
          opcode.addImm(0b100000, 0);
        }
        opcode.addReg(o0, opData.rShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLIR: {
      const InstDB::EncodingData::BaseLIR& opData =
          InstDB::EncodingData::baseLIR[encodingIndex];

      if (isign4 == ENC_OPS2(Imm, Reg)) {
        if (!checkGpType(o1, opData.aType)) goto InvalidInstruction;

        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();

        if (aImm > 7) goto InvalidImmediate;
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.aShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLIV: {
      const InstDB::EncodingData::BaseLIV& opData =
          InstDB::EncodingData::baseLIV[encodingIndex];

      if (isign4 == ENC_OPS2(Imm, Reg)) {
        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();
        if (aImm > 7) goto InvalidImmediate;

        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.aShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }
      break;
    }

    case InstDB::kEncodingBaseLVI: {
      const InstDB::EncodingData::BaseLVI& opData =
          InstDB::EncodingData::baseLVI[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        if (aImm > 7) goto InvalidImmediate;

        opcode.reset(opData.opcode());
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }
      break;
    }

    case InstDB::kEncodingBaseLRR: {
      const InstDB::EncodingData::BaseLRR& opData =
          InstDB::EncodingData::baseLRR[encodingIndex];
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;
        if (opData.uniform == 0 && !checkSignature(o0, o1))
          goto InvalidInstruction;
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLVV: {
      const InstDB::EncodingData::BaseLVV& opData =
          InstDB::EncodingData::baseLVV[encodingIndex];
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLVR: {
      const InstDB::EncodingData::BaseLVR& opData =
          InstDB::EncodingData::baseLVR[encodingIndex];
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o1, opData.bType)) {
          goto InvalidInstruction;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRV: {
      const InstDB::EncodingData::BaseLRV& opData =
          InstDB::EncodingData::baseLRV[encodingIndex];
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o0, opData.aType)) {
          goto InvalidInstruction;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRL: {
      const InstDB::EncodingData::BaseLRRL& opData =
          InstDB::EncodingData::baseLRRL[encodingIndex];
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRR: {
      const InstDB::EncodingData::BaseLRRR& opData =
          InstDB::EncodingData::baseLRRR[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        if (opData.uniform == 0 && !checkSignature(o0, o1, o2))
          goto InvalidInstruction;

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRRT: {
      const InstDB::EncodingData::BaseLRRRT& opData =
          InstDB::EncodingData::baseLRRRT[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        if (opData.uniform == 0 && !checkSignature(o0, o1, o2))
          goto InvalidInstruction;

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLVRR: {
      const InstDB::EncodingData::BaseLVRR& opData =
          InstDB::EncodingData::baseLVRR[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        if (opData.uniform == 0 && !checkSignature(o1, o2))
          goto InvalidInstruction;

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLVVV: {
      const InstDB::EncodingData::BaseLVVV& opData =
          InstDB::EncodingData::baseLVVV[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (opData.uniform == 0 && !checkSignature(o0, o1, o2))
          goto InvalidInstruction;

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRRI: {
      const InstDB::EncodingData::BaseLRRRI& opData =
          InstDB::EncodingData::baseLRRRI[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        uint32_t aImm = o3.as<Imm>().valueAs<uint32_t>();

        if ((aImm) != o3.as<Imm>().valueAs<uint32_t>()) goto InvalidImmediate;

        if ((aImm < 1 || aImm > 4) && opData.uniform != 2) {
          goto InvalidInstruction;
        }

        if ((aImm > 7) && opData.uniform == 2) {
          goto InvalidInstruction;
        }

        if (opData.uniform == 0) {
          aImm--;
        }
        opcode.reset(opData.opcode());
        uint32_t ik = 1;
        if (opData.uniform == 2) {
          opcode.shiftopL(ik);
        }
        opcode.addImm(aImm, opData.aImmOffset);

        opcode.addReg(o2, 10);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      }

      break;
    }
    case InstDB::kEncodingBaseLRRII: {
      const InstDB::EncodingData::BaseLRRII& opData =
          InstDB::EncodingData::baseLRRII[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t bImm = o2.as<Imm>().valueAs<uint32_t>();
        uint32_t aImm = o3.as<Imm>().valueAs<uint32_t>();

        if ((aImm > 31 || bImm > 31 || aImm > bImm) &&
            (opData.uniform == 0 || opData.uniform == 1)) {
          goto InvalidImmediate;
        }
        if ((aImm > 63 || bImm > 63 || aImm > bImm) && opData.uniform == 2) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1) {
          aImm |= 0x20;
        }
        opcode.reset(opData.opcode());
        uint32_t ik = 1;
        if (opData.uniform == 2) {
          opcode.shiftopL(ik);
        }

        opcode.addImm(bImm, opData.bImmOffset);
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRI: {
      const InstDB::EncodingData::BaseLRRI& opData =
          InstDB::EncodingData::baseLRRI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();

        int32_t amk = o2.as<Imm>().valueAs<int32_t>();
        if (((aImm >> 12) > 0) && opData.uniform == 0) {
          aImm &= 0xFFF;
        }

        if (opData.uniform == 0 && (amk > 2047 || amk < -2048)) { //u12
          goto InvalidImmediate;
        }
        if (opData.uniform == 1 && (aImm > 31)) {
          goto InvalidImmediate;
        }
        if (opData.uniform == 2 && (aImm > 63)) {
          goto InvalidImmediate;
        }
        if (opData.uniform == 3 && ((aImm >> 14) > 0)) {
          goto InvalidImmediate;
        }
        if (opData.uniform == 4 && ((aImm >> 8) > 0)) {
          goto InvalidImmediate;
        }
        if (opData.uniform == 5 && ((aImm >> 12) > 0))  // ui12
        {
          goto InvalidImmediate;
        }
        opcode.reset(opData.opcode());
        if ((opData.uniform == 0) || (opData.uniform == 5))  // si12 or ui12
        {
          opcode.shiftopL(7);
        }
        if (opData.uniform == 2) {
          opcode.shiftopL(1);
        }
        if (opData.uniform == 3) {
          opcode.shiftopL(9);
        }
        if (opData.uniform == 4) {
          opcode.shiftopL(3);
        }
        if (opData.uniform == 3 && (o1._baseId == 0 || o1._baseId == 1)) {
          goto InvalidInstruction;
        }
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLIRR: {
      const InstDB::EncodingData::BaseLIRR& opData =
          InstDB::EncodingData::baseLIRR[encodingIndex];

      if (isign4 == ENC_OPS3(Imm, Reg, Reg)) {
        if (!checkGpType(o1, opData.aType)) goto InvalidInstruction;

        if (!checkGpType(o2, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();

        if (opData.uniform == 0 && (aImm > 31)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.bShift);
        opcode.addReg(o1, opData.aShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLFIVV: {
      const InstDB::EncodingData::BaseLFIVV& opData =
          InstDB::EncodingData::baseLFIVV[encodingIndex];

      if (isign4 == ENC_OPS3(Imm, Reg, Reg)) {
        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();

        if (opData.uniform == 0 && (aImm > 7)) {
          goto InvalidInstruction;
        }

        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLRRIL: {
      const InstDB::EncodingData::BaseLRRIL& opData =
          InstDB::EncodingData::baseLRRIL[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkGpId(o0, opData.aHiId)) goto InvalidPhysId;
        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();

        if ((aImm) != o2.as<Imm>().valueAs<uint32_t>()) goto InvalidImmediate;
        if ((aImm >> 16) > 0) {
          aImm &= 0xFFFF;
        }
        opcode.reset(opData.opcode());
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLII: {
      const InstDB::EncodingData::BaseLII& opData =
          InstDB::EncodingData::baseLII[encodingIndex];

      if (isign4 == ENC_OPS2(Imm, Imm)) {
        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        uint32_t bImm = o0.as<Imm>().valueAs<uint32_t>();
        uint32_t cImm;
        int64_t ka = o1.as<Imm>().valueAs<int64_t>();

        if ((aImm & 0x3u) != 0) {
          printf("The lowest double bit must be zero\n");
          goto InvalidImmediate;
        }

        if ((ka < -1 * (1 << 22)) || (ka > 0x3fffff)) {
          goto InvalidImmediate;
        }

        if (bImm >= uint64_t(1u << opData.bImmSize)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1) {
          bImm ^= 0x8;
        }
        cImm = ((aImm >> 2) & 0xFFFF);
        aImm >>= 18;
        aImm &= 0x1F;
        opcode.reset(opData.opcode());
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addImm(bImm, opData.bImmOffset);
        opcode.addImm(cImm, opData.cImmOffset);
        goto EmitOp;
      }

      break;
    }
    case InstDB::kEncodingBaseBranchRel: {
      const InstDB::EncodingData::BaseBranchRel& opData =
          InstDB::EncodingData::baseBranchRel[encodingIndex];

      if (isign4 == ENC_OPS1(Label) || isign4 == ENC_OPS1(Imm)) {
        opcode.reset(opData.opcode());
        rmRel = &o0;

        offsetFormat.resetToImmValue(OffsetType::kTypeLa64_B26, 4, 0, 26,
                                     2);  // B and BL
        goto EmitOp_Rel;
      } else if (isign4 == ENC_OPS3(Reg, Reg, Imm) ||
                 isign4 == ENC_OPS3(Reg, Reg, Label)) {
        opcode.reset(opData.opcode());
        rmRel = &o2;

        opcode.addReg(o0, 5);
        opcode.addReg(o1, 0);

        offsetFormat.resetToImmValue(OffsetType::kTypeLa64_B16, 4, 10, 16,
                                     2);  // BEQ ...
        goto EmitOp_Rel;
      }

      break;
    }
      /* case InstDB::kEncodingBTLRRI: { */

    case InstDB::kEncodingJBTLRRI: {
      const InstDB::EncodingData::JBTLRRI& opData =
          InstDB::EncodingData::jBTLRRI[encodingIndex];
      if (isign4 == ENC_OPS3(Reg, Reg, Imm) ||
          isign4 == ENC_OPS3(Reg, Reg, Label)) {
        if (!checkGpId(o0, o1, kZR)) goto InvalidPhysId;

        int32_t imm = o2.as<Imm>().valueAs<int32_t>();

        if ((imm > 0x1FFFF) || imm < -1 * (1 << 17)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addReg(o0, 0);
        opcode.addReg(o1, 5);
        imm &= 0x3FFFF;
        imm >>= 2;
        opcode.addImm(imm, 10);
        goto EmitOp;
      }

      break;
    }

      /* case InstDB::kEncodingBaseLldst: { */

    case InstDB::kEncodingBaseLdSt: {
      const InstDB::EncodingData::BaseLdSt& opData =
          InstDB::EncodingData::baseLdSt[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rmRel = &m;

        if (!checkGpId(o0, kZR)) goto InvalidPhysId;

        if (!laCheckMemBaseIndexRel(m)) goto InvalidAddress;

        int64_t offset = m.offset();
        if (m.hasBaseReg()) {
          // [Base {Offset | Index}]
          if (m.hasIndex()) {
            opcode.reset(opData.opcode());
            opcode.addReg(o0, 0);
            goto EmitOp_MemBaseIndex_Rj5_Rk10;
          }

          // Makes it easier to work with the offset especially on 32-bit arch.
          if (!Support::isInt32(offset)) goto InvalidDisplacement;
          int32_t offset32 = int32_t(offset);

          if (!Support::isInt12(offset32)) goto InvalidDisplacement;

          opcode.reset(opData.opcode());
          opcode.addImm(offset32 & 0xFFF, 10);
          opcode.addReg(o0, 0);
          goto EmitOp_MemBase_Rj5;
        } else {
          goto InvalidAddress;
        }
      } else if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        // TODO: add reg type check
        int64_t immVal = o2.as<Imm>().valueAs<int64_t>();

        // Makes it easier to work with the offset especially on 32-bit arch.
        if (!Support::isInt32(immVal)) goto InvalidDisplacement;

        int32_t offset32 = int32_t(immVal);
        if (!Support::isInt12(offset32)) goto InvalidDisplacement;

        uint32_t offset = offset32 & 0xFFF;
        opcode.reset(opData.opcode());
        opcode.addImm(offset, 10);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      } else if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        // TODO: add instr check
        opcode.reset(opData.opcode());
        opcode.addReg(o2, 10);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingSimdLdst: {
      const InstDB::EncodingData::SimdLdst& opData =
          InstDB::EncodingData::simdLdst[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rmRel = &m;

        if (!laCheckMemBaseIndexRel(m)) goto InvalidAddress;

        int64_t offset = m.offset();
        if (m.hasBaseReg()) {
          // [Base {Offset | Index}]
          if (m.hasIndex()) {
            opcode.reset(opData.opcode());
            opcode.addReg(o0, 0);
            goto EmitOp_MemBaseIndex_Rj5_Rk10;
          }

          // Makes it easier to work with the offset especially on 32-bit arch.
          if (!Support::isInt32(offset)) goto InvalidDisplacement;
          int32_t offset32 = int32_t(offset);

          if (!Support::isInt12(offset32)) goto InvalidDisplacement;

          uint32_t offsetLen = opData.offsetLen;
          uint32_t offset = 0;
          switch (offsetLen) {
            case 9:
              if ((offset32 & 0x7) != 0) goto InvalidDisplacement;
              offset = (offset32 >> 3) & 0x1FF;
              break;
            case 10:
              if ((offset32 & 0x3) != 0) goto InvalidDisplacement;
              offset = (offset32 >> 2) & 0x3FF;
              break;
            case 11:
              if ((offset32 & 0x1) != 0) goto InvalidDisplacement;
              offset = (offset32 >> 1) & 0x7FF;
              break;
            case 12:
              offset = offset32 & 0xFFF;
              break;
            default:
              goto InvalidDisplacement;
          }

          opcode.reset(opData.opcode());
          opcode.addImm(offset, 10);
          opcode.addReg(o0, 0);
          goto EmitOp_MemBase_Rj5;
        } else {
          goto InvalidAddress;
        }
      } else if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        // TODO: add reg type check
        int64_t immVal = o2.as<Imm>().valueAs<int64_t>();

        // Makes it easier to work with the offset especially on 32-bit arch.
        if (!Support::isInt32(immVal)) goto InvalidDisplacement;

        int32_t offset32 = int32_t(immVal);
        if (!Support::isInt12(offset32)) goto InvalidDisplacement;

        uint32_t offsetLen = opData.offsetLen;
        uint32_t offset = 0;
        switch (offsetLen) {
          case 9:
            if ((offset32 & 0x7) != 0) goto InvalidDisplacement;
            offset = (offset32 >> 3) & 0x1FF;
            break;
          case 10:
            if ((offset32 & 0x3) != 0) goto InvalidDisplacement;
            offset = (offset32 >> 2) & 0x3FF;
            break;
          case 11:
            if ((offset32 & 0x1) != 0) goto InvalidDisplacement;
            offset = (offset32 >> 1) & 0x7FF;
            break;
          case 12:
            offset = offset32 & 0xFFF;
            break;
          default:
            goto InvalidDisplacement;
        }

        opcode.reset(opData.opcode());
        opcode.addImm(offset, 10);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      } else if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        uint32_t offsetLen = opData.offsetLen;
        if (0 != offsetLen) goto InvalidDisplacement;

        opcode.reset(opData.opcode());
        opcode.addReg(o2, 10);
        opcode.addReg(o1, 5);
        opcode.addReg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingFpLVVVV: {
      const InstDB::EncodingData::FpLVVVV& opData =
          InstDB::EncodingData::fpLVVVV[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        if (!matchSignature(o0, o1, o2, o3, instFlags)) goto InvalidInstruction;

        // goto EmitOp_Rd0_Rn5_Rm16_Ra10;
        if (!checkValidRegs(o0, o1, o2, o3)) goto InvalidPhysId;
        opcode.reset(opData.opcode());
        opcode.addReg(o0, 0);
        opcode.addReg(o1, 5);
        opcode.addReg(o2, 10);
        opcode.addReg(o3, 15);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVVVV: {
      const InstDB::EncodingData::LsxVVVV& opData =
          InstDB::EncodingData::lsxVVVV[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        if (!matchSignature(o0, o1, o2, o3, instFlags)) goto InvalidInstruction;

        // goto EmitOp_Rd0_Rn5_Rm16_Ra10;
        if (!checkValidRegs(o0, o1, o2, o3)) goto InvalidPhysId;
        opcode.reset(opData.opcode());
        opcode.addReg(o0, opData.aShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o3, opData.dShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVVV: {
      const InstDB::EncodingData::LsxVVV& opData =
          InstDB::EncodingData::lsxVVV[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!matchSignature(o0, o1, o2, instFlags)) {
          goto InvalidInstruction;
        }

        // goto EmitOp_Rd0_Rn5_Rm16_Ra10;
        if (!checkValidRegs(o0, o1, o2)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o0, opData.aShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o2, opData.cShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXXX: {
      const InstDB::EncodingData::LasxXXX& opData =
          InstDB::EncodingData::lasxXXX[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!matchSignature(o0, o1, o2, instFlags)) {
          goto InvalidInstruction;
        }

        if (!checkValidRegs(o0, o1, o2)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o0, opData.aShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o2, opData.cShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXXXX: {
      const InstDB::EncodingData::LasxXXXX& opData =
          InstDB::EncodingData::lasxXXXX[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        if (!matchSignature(o0, o1, o2, o3, instFlags)) goto InvalidInstruction;

        if (!checkValidRegs(o0, o1, o2, o3)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o0, opData.aShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o3, opData.dShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVRI: {
      const InstDB::EncodingData::LsxVRI& opData =
          InstDB::EncodingData::lsxVRI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o2.as<Imm>().valueAs<int64_t>();

        if ((immValue > 2047 || immValue < (-2048)) &&
            (opData.uniform == 0 || opData.uniform == 4)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF007) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF003) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF001) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 5 && (immValue > 15 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 6 && (immValue > 7 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 7 && (immValue > 3 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 8 && (immValue > 1 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1) {
          aImm >>= 3;
          immValue >>= 3;
        }

        if (opData.uniform == 2) {
          aImm >>= 2;
          immValue >>= 2;
        }

        if (opData.uniform == 3) {
          aImm >>= 1;
          immValue >>= 1;
        }

        opcode.reset(opData.opcode());
        if (opData.uniform == 0 || opData.uniform == 4) {
          opcode.shiftopL(11);
        }
        if (opData.uniform == 1) {
          opcode.shiftopL(8);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(9);
        }

        if (opData.uniform == 3) {
          opcode.shiftopL(10);
        }

        if (opData.uniform == 5) {
          opcode.shiftopL(3);
        }

        if (opData.uniform == 6) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 7) {
          opcode.shiftopL(1);
        }

        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }
        if ((immValue < 0 && opData.uniform == 0) || opData.uniform == 4) {
          aImm &= 0xFFF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 1) {
          aImm &= 0x1FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 2) {
          aImm &= 0x3FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 3) {
          aImm &= 0x7FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXRI: {
      const InstDB::EncodingData::LasxXRI& opData =
          InstDB::EncodingData::lasxXRI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o2.as<Imm>().valueAs<int64_t>();

        if ((immValue > 2047 || immValue < (-2048)) &&
            (opData.uniform == 0 || opData.uniform == 4)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF007) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF003) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 &&
            (((immValue + 0x800) & 0xFFFFFFFFFFFFF001) != 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 5 && (immValue > 7 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 6 && (immValue > 3 || immValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1) {
          aImm >>= 3;
          immValue >>= 3;
        }

        if (opData.uniform == 2) {
          aImm >>= 2;
          immValue >>= 2;
        }

        if (opData.uniform == 3) {
          aImm >>= 1;
          immValue >>= 1;
        }

        opcode.reset(opData.opcode());
        if (opData.uniform == 0 || opData.uniform == 4) {
          opcode.shiftopL(10);
        }
        if (opData.uniform == 1) {
          opcode.shiftopL(7);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(8);
        }

        if (opData.uniform == 3) {
          opcode.shiftopL(9);
        }

        if (opData.uniform == 5) {
          opcode.shiftopL(1);
        }

        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }
        if ((immValue < 0 && opData.uniform == 0) || opData.uniform == 4) {
          aImm &= 0xFFF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 1) {
          aImm &= 0x1FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 2) {
          aImm &= 0x3FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue < 0 && opData.uniform == 3) {
          aImm &= 0x7FF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVRII: {
      const InstDB::EncodingData::LsxVRII& opData =
          InstDB::EncodingData::lsxVRII[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        uint32_t bImm = o3.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o2.as<Imm>().valueAs<int64_t>();
        int64_t bimmValue = o3.as<Imm>().valueAs<int64_t>();

        if (((aimmValue >> 3) > 127 || (aimmValue >> 3) < (-128)) &&
            opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((bimmValue > 1 || bimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 &&
            ((aimmValue >> 2) > 127 || (aimmValue >> 2) < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 && (bimmValue > 3 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 &&
            ((aimmValue >> 1) > 127 || (aimmValue >> 1) < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 && (bimmValue > 7 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (aimmValue > 127 || aimmValue < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (bimmValue > 15 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 0) {
          aImm >>= 3;
          aimmValue >>= 3;
        }

        if (opData.uniform == 1) {
          aImm >>= 2;
          aimmValue >>= 2;
        }

        if (opData.uniform == 2) {
          aImm >>= 1;
          aimmValue >>= 1;
        }

        opcode.reset(opData.opcode());

        if (opData.uniform == 1) {
          opcode.shiftopL(1);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 3) {
          opcode.shiftopL(3);
        }

        opcode.addImm(bImm, opData.bImmOffset);

        if (aimmValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (aimmValue < 0) {
          aImm &= 0xFF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXRII: {
      const InstDB::EncodingData::LasxXRII& opData =
          InstDB::EncodingData::lasxXRII[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        uint32_t bImm = o3.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o2.as<Imm>().valueAs<int64_t>();
        int64_t bimmValue = o3.as<Imm>().valueAs<int64_t>();

        if (((aimmValue >> 3) > 127 || (aimmValue >> 3) < (-128)) &&
            opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((bimmValue > 3 || bimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 &&
            ((aimmValue >> 2) > 127 || (aimmValue >> 2) < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 && (bimmValue > 7 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 &&
            ((aimmValue >> 1) > 127 || (aimmValue >> 1) < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 && (bimmValue > 15 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (aimmValue > 127 || aimmValue < (-128))) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (bimmValue > 31 || bimmValue < 0)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 0) {
          aImm >>= 3;
          aimmValue >>= 3;
        }

        if (opData.uniform == 1) {
          aImm >>= 2;
          aimmValue >>= 2;
        }

        if (opData.uniform == 2) {
          aImm >>= 1;
          aimmValue >>= 1;
        }

        opcode.reset(opData.opcode());

        if (opData.uniform == 1) {
          opcode.shiftopL(1);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 3) {
          opcode.shiftopL(3);
        }

        opcode.addImm(bImm, opData.bImmOffset);

        if (aimmValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (aimmValue < 0) {
          aImm &= 0xFF;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

      /* case InstDB::kEncodingLsxVRR: { */

      /* case InstDB::kEncodingLasxXRR: { */

    case InstDB::kEncodingLsxVVR: {
      const InstDB::EncodingData::LsxVVR& opData =
          InstDB::EncodingData::lsxVVR[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXXR: {
      const InstDB::EncodingData::LasxXXR& opData =
          InstDB::EncodingData::lasxXXR[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!checkGpType(o2, opData.cType)) goto InvalidInstruction;

        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o2, opData.cShift);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVVI: {
      const InstDB::EncodingData::LsxVVI& opData =
          InstDB::EncodingData::lsxVVI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o2.as<Imm>().valueAs<int64_t>();

        if (opData.uniform == 0 && (immValue < (-16) || immValue > 15)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 9 && (immValue < 0 || immValue > 31)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 8 && (immValue < 0 || immValue > 255)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 7 && (immValue < 0 || immValue > 127)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 && (immValue < 0 || immValue > 7)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 && (immValue < 0 || immValue > 15)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (immValue < 0 || immValue > 31)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 4 && (immValue < 0 || immValue > 63)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 5 && (immValue < 0 || immValue > 3)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 6 && (immValue < 0 || immValue > 1)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());

        if (opData.uniform == 0 || opData.uniform == 3 || opData.uniform == 9) {
          opcode.shiftopL(4);
        }

        if (opData.uniform == 8) {
          opcode.shiftopL(7);
        }

        if (opData.uniform == 7) {
          opcode.shiftopL(6);
        }

        if (opData.uniform == 5) {
          opcode.shiftopL(1);
        }

        if (opData.uniform == 1) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(3);
        }

        if (opData.uniform == 4) {
          opcode.shiftopL(5);
        }

        if (immValue < 0 && opData.uniform == 0) {
          aImm &= 0x1F;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXXI: {
      const InstDB::EncodingData::LasxXXI& opData =
          InstDB::EncodingData::lasxXXI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o2.as<Imm>().valueAs<int64_t>();

        if (opData.uniform == 0 && (immValue < (-16) || immValue > 15)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 9 && (immValue < 0 || immValue > 31)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 8 && (immValue < 0 || immValue > 255)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 7 && (immValue < 0 || immValue > 127)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 1 && (immValue < 0 || immValue > 7)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 2 && (immValue < 0 || immValue > 15)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 3 && (immValue < 0 || immValue > 31)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 4 && (immValue < 0 || immValue > 63)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 5 && (immValue < 0 || immValue > 3)) {
          goto InvalidImmediate;
        }

        if (opData.uniform == 6 && (immValue < 0 || immValue > 1)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());

        if (opData.uniform == 0 || opData.uniform == 3 || opData.uniform == 9) {
          opcode.shiftopL(4);
        }

        if (opData.uniform == 8) {
          opcode.shiftopL(7);
        }

        if (opData.uniform == 7) {
          opcode.shiftopL(6);
        }

        if (opData.uniform == 5) {
          opcode.shiftopL(1);
        }

        if (opData.uniform == 1) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 2) {
          opcode.shiftopL(3);
        }

        if (opData.uniform == 4) {
          opcode.shiftopL(5);
        }

        if (immValue < 0 && opData.uniform == 0) {
          aImm &= 0x1F;
          opcode.addImm(aImm, opData.aImmOffset);
        }

        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }

        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVII: {
      const InstDB::EncodingData::LsxVII& opData =
          InstDB::EncodingData::lsxVII[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        uint32_t bImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o1.as<Imm>().valueAs<int64_t>();
        int64_t bimmValue = o2.as<Imm>().valueAs<int64_t>();
        if ((aimmValue > 31 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((bimmValue > 31 || bimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addImm(bImm, opData.bImmOffset);
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXII: {
      const InstDB::EncodingData::LasxXII& opData =
          InstDB::EncodingData::lasxXII[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        uint32_t bImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o1.as<Imm>().valueAs<int64_t>();
        int64_t bimmValue = o2.as<Imm>().valueAs<int64_t>();
        if ((aimmValue > 31 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((bimmValue > 31 || bimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addImm(bImm, opData.bImmOffset);
        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVV: {
      const InstDB::EncodingData::LsxVV& opData =
          InstDB::EncodingData::lsxVV[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXX: {
      const InstDB::EncodingData::LasxXX& opData =
          InstDB::EncodingData::lasxXX[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkValidRegs(o0, o1)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxIV: {
      const InstDB::EncodingData::LsxIV& opData =
          InstDB::EncodingData::lsxIV[encodingIndex];

      if (isign4 == ENC_OPS2(Imm, Reg)) {
        if (!checkValidRegs(o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o0.as<Imm>().valueAs<int64_t>();
        if ((aimmValue > 7 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.aShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxIX: {
      const InstDB::EncodingData::LasxIX& opData =
          InstDB::EncodingData::lasxIX[encodingIndex];

      if (isign4 == ENC_OPS2(Imm, Reg)) {
        if (!checkValidRegs(o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o0.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o0.as<Imm>().valueAs<int64_t>();
        if ((aimmValue > 7 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.aShift);
        opcode.addImm(aImm, opData.aImmOffset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVR: {
      const InstDB::EncodingData::LsxVR& opData =
          InstDB::EncodingData::lsxVR[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXR: {
      const InstDB::EncodingData::LasxXR& opData =
          InstDB::EncodingData::lasxXR[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!checkGpType(o1, opData.bType)) goto InvalidInstruction;

        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }
        opcode.reset(opData.opcode());
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxRVI: {
      const InstDB::EncodingData::LsxRVI& opData =
          InstDB::EncodingData::lsxRVI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkValidRegs(o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o2.as<Imm>().valueAs<int64_t>();

        if ((aimmValue > 15 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((aimmValue > 7 || aimmValue < 0) && opData.uniform == 1) {
          goto InvalidImmediate;
        }

        if ((aimmValue > 3 || aimmValue < 0) && opData.uniform == 2) {
          goto InvalidImmediate;
        }

        if ((aimmValue > 1 || aimmValue < 0) && opData.uniform == 3) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        if (opData.uniform == 2) {
          opcode.shiftopL(1);
        }

        if (opData.uniform == 1) {
          opcode.shiftopL(2);
        }

        if (opData.uniform == 0) {
          opcode.shiftopL(3);
        }

        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxRXI: {
      const InstDB::EncodingData::LasxRXI& opData =
          InstDB::EncodingData::lasxRXI[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!checkGpType(o0, opData.aType)) goto InvalidInstruction;

        if (!checkValidRegs(o1)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o2.as<Imm>().valueAs<uint32_t>();
        int64_t aimmValue = o2.as<Imm>().valueAs<int64_t>();

        if ((aimmValue > 7 || aimmValue < 0) && opData.uniform == 0) {
          goto InvalidImmediate;
        }

        if ((aimmValue > 3 || aimmValue < 0) && opData.uniform == 1) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        if (opData.uniform == 0) {
          opcode.shiftopL(1);
        }

        opcode.addImm(aImm, opData.aImmOffset);
        opcode.addReg(o1, opData.bShift);
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLsxVI: {
      const InstDB::EncodingData::LsxVI& opData =
          InstDB::EncodingData::lsxVI[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o1.as<Imm>().valueAs<int64_t>();

        if (immValue > 4095 || immValue < (-4096)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }
        if (immValue < 0) {
          immValue &= 0x1FFF;
          opcode.addImm(immValue, opData.aImmOffset);
        }
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLasxXI: {
      const InstDB::EncodingData::LasxXI& opData =
          InstDB::EncodingData::lasxXI[encodingIndex];

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (!checkValidRegs(o0)) {
          goto InvalidPhysId;
        }

        uint32_t aImm = o1.as<Imm>().valueAs<uint32_t>();
        int64_t immValue = o1.as<Imm>().valueAs<int64_t>();

        if (immValue > 4095 || immValue < (-4096)) {
          goto InvalidImmediate;
        }

        opcode.reset(opData.opcode());
        if (immValue >= 0) {
          opcode.addImm(aImm, opData.aImmOffset);
        }
        if (immValue < 0) {
          immValue &= 0x1FFF;
          opcode.addImm(immValue, opData.aImmOffset);
        }
        opcode.addReg(o0, opData.aShift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingLfVVVI: {
      const InstDB::EncodingData::LfVVVI& opData =
          InstDB::EncodingData::lfVVVI[encodingIndex];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!matchSignature(o0, o1, o2, instFlags)) goto InvalidInstruction;

        uint64_t immValue = o3.as<Imm>().valueAs<uint64_t>();
        uint32_t immSize = opData.immSize;

        if (immValue >= (1u << immSize)) goto InvalidImmediate;

        opcode.reset(opData.opcode());
        opcode.addImm(immValue, opData.immShift);
        opcode.addReg(o2, opData.cshift);
        opcode.addReg(o1, opData.bshift);
        opcode.addReg(o0, opData.ashift);
        goto EmitOp;
      }

      break;
    }

      /* case InstDB::kEncodingLFldst: { */

    case InstDB::kEncodingLPldst: {
      const InstDB::EncodingData::LPldst& opData =
          InstDB::EncodingData::lPldst[encodingIndex];

      if (isign4 == ENC_OPS3(Imm, Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rmRel = &m;

        if (!checkVecId(o0, o1)) goto InvalidPhysId;

        if (m.baseType() != RegType::kLA_GpX || m.hasIndex())
          goto InvalidAddress;

        if (m.isOffset64Bit()) goto InvalidDisplacement;
        opcode.reset(uint32_t(opData.offsetOp) << 22);
        uint32_t kk;
        if (o2._data[1] > 2047) {
          kk = o2._data[1] & 0xFFF;
        } else {
          kk = o2._data[1];
        }
        opcode.addImm(kk, 10);
        goto EmitOp_MemBase_Rn5;
      }

      break;
    }

    case InstDB::kEncodingLCldst: {
      const InstDB::EncodingData::LCldst& opData =
          InstDB::EncodingData::lCldst[encodingIndex];

      if (isign4 == ENC_OPS3(Imm, Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rmRel = &m;

        if (!checkVecId(o0, o1)) goto InvalidPhysId;

        if (m.baseType() != RegType::kLA_GpX || m.hasIndex())
          goto InvalidAddress;

        if (m.isOffset64Bit()) goto InvalidDisplacement;
        opcode.reset(uint32_t(opData.offsetOp) << 22);
        uint32_t kk;
        if (o2._data[1] > 2047) {
          kk = o2._data[1] & 0xFFF;
        } else {
          kk = o2._data[1];
        }

        opcode.addImm(kk, 10);
        opcode.addImm(o0._data[0], 0);
        goto EmitOp_MemBase_Rn5;
      }

      break;
    }

    case InstDB::kEncodingLFPldst: {
      const InstDB::EncodingData::LFPldst& opData =
          InstDB::EncodingData::lFPldst[encodingIndex];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rmRel = &m;

        if (!checkVecId(o0, o1)) goto InvalidPhysId;

        if (m.baseType() != RegType::kLA_GpX || m.hasIndex())
          goto InvalidAddress;

        if (m.isOffset64Bit()) goto InvalidDisplacement;
        opcode.reset(uint32_t(opData.offsetOp) << 24);
        if ((o2._data[1] & 0x3) != 0) {
          printf("require imm low 2 bit is 0.");
          goto EmitOp;
        }
        uint32_t kk;
        if (o2._data[1] > 32764) {
          kk = o2._data[1] & 0xFFFF;
        } else {
          kk = o2._data[1];
        }
        kk >>= 2;

        opcode.addImm(kk, 10);
        opcode.addReg(o0, 0);
        goto EmitOp_MemBase_Rn5;
      }

      break;
    }
    default:
      break;
  }

  goto InvalidInstruction;

  // --------------------------------------------------------------------------
  // [EmitGp - Single]
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // [EmitGp - Memory]
  // --------------------------------------------------------------------------

EmitOp_MemBase_Rn5:
  if (!checkMemBase(rmRel->as<Mem>())) goto InvalidAddress;

  opcode.addReg(rmRel->as<Mem>().baseId(), 5);
  goto EmitOp;

EmitOp_MemBase_Rj5:
  if (!checkMemBase(rmRel->as<Mem>())) goto InvalidAddress;

  opcode.addReg(rmRel->as<Mem>().baseId(), 5);
  goto EmitOp;

EmitOp_MemBaseIndex_Rj5_Rk10:
  if (!rmRel->as<Mem>().hasBaseReg()) goto InvalidAddress;

  opcode.addReg(rmRel->as<Mem>().indexId(), 10);
  opcode.addReg(rmRel->as<Mem>().baseId(), 5);
  goto EmitOp;

  // --------------------------------------------------------------------------
  // [EmitOp - PC Relative]
  // --------------------------------------------------------------------------

EmitOp_Rel: {
  if (rmRel->isLabel() || rmRel->isMem()) {
    uint32_t labelId;
    int64_t labelOffset = 0;

    if (rmRel->isLabel()) {
      labelId = rmRel->as<Label>().id();
    } else {
      labelId = rmRel->as<Mem>().baseId();
      labelOffset = rmRel->as<Mem>().offset();
    }

    LabelEntry* label = _code->labelEntry(labelId);
    if (ASMJIT_UNLIKELY(!label)) goto InvalidLabel;

    if (label->isBoundTo(_section)) {
      // Label bound to the current section.
      offsetValue =
          label->offset() - uint64_t(offset()) + uint64_t(labelOffset);
      goto EmitOp_DispImm;
    } else {
      // Record non-bound label.
      size_t codeOffset = writer.offsetFrom(_bufferData);
      LabelLink* link =
          _code->newLabelLink(label, _section->id(), codeOffset,
                              intptr_t(labelOffset), offsetFormat);

      if (ASMJIT_UNLIKELY(!link)) goto OutOfMemory;

      goto EmitOp;
    }
  }
}

  if (rmRel->isImm()) {
    uint64_t baseAddress = _code->baseAddress();
    uint64_t targetOffset = rmRel->as<Imm>().valueAs<uint64_t>();

    size_t codeOffset = writer.offsetFrom(_bufferData);

    if (baseAddress == Globals::kNoBaseAddress || _section->id() != 0) {
      // Create a new RelocEntry as we cannot calculate the offset right now.
      RelocEntry* re;
      err = _code->newRelocEntry(&re, RelocType::kAbsToRel);
      if (err) goto Failed;

      re->_sourceSectionId = _section->id();
      re->_sourceOffset = codeOffset;
      re->_format = offsetFormat;
      re->_payload = rmRel->as<Imm>().valueAs<uint64_t>() + 4u;
      goto EmitOp;
    } else {
      uint64_t pc = baseAddress + codeOffset;

      offsetValue = targetOffset - pc;
      goto EmitOp_DispImm;
    }
  }

  goto InvalidInstruction;

EmitOp_DispImm: {
  if ((offsetValue &
       Support::lsbMask<uint32_t>(offsetFormat.immDiscardLsb())) != 0)
    goto InvalidDisplacement;

  int64_t dispImm64 = int64_t(offsetValue) >> offsetFormat.immDiscardLsb();
  if (!Support::isEncodableOffset64(dispImm64, offsetFormat.immBitCount()))
    goto InvalidDisplacement;

  uint32_t dispImm32 = uint32_t(
      dispImm64 & Support::lsbMask<uint32_t>(offsetFormat.immBitCount()));
  switch (offsetFormat.type()) {
    case OffsetType::kSignedOffset: {
      opcode.addImm(dispImm32, offsetFormat.immBitShift());
      goto EmitOp;
    }

    default:
      goto InvalidDisplacement;
  }
}

  // --------------------------------------------------------------------------
  // [EmitOp - Opcode]
  // --------------------------------------------------------------------------

EmitOp:
  writer.emit32uLE(opcode.get());
  goto EmitDone;

  // --------------------------------------------------------------------------
  // [Done]
  // --------------------------------------------------------------------------

EmitDone:
  if (Support::test(options, InstOptions::kReserved)) {
#ifndef ASMJIT_NO_LOGGING
    if (_logger)
      EmitterUtils::logInstructionEmitted(
          this, BaseInst::composeLAInstId(instId, instCC), options, o0, o1, o2,
          opExt, 0, 0, writer.cursor());
#endif
  }

  resetState();

  writer.done(this);
  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Error Handler]
  // --------------------------------------------------------------------------

#define ERROR_HANDLER(ERR)                \
  ERR:                                    \
  err = DebugUtils::errored(kError##ERR); \
  goto Failed;
  ERROR_HANDLER(OutOfMemory)
  ERROR_HANDLER(InvalidAddress)
  ERROR_HANDLER(InvalidDisplacement)
  ERROR_HANDLER(InvalidLabel)
  ERROR_HANDLER(InvalidImmediate)
  ERROR_HANDLER(InvalidInstruction)
  ERROR_HANDLER(InvalidPhysId)
#undef ERROR_HANDLER

Failed:
#ifndef ASMJIT_NO_LOGGING
  return EmitterUtils::logInstructionFailed(this, err, instId, options, o0, o1,
                                            o2, opExt);
#else
  resetState();
  return reportError(err);
#endif
}

#undef ENC_OPS1
#undef ENC_OPS2
#undef ENC_OPS3
#undef ENC_OPS4

// a64::Assembler - Align
// ======================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  constexpr uint32_t kNopA64 =
      0xD503201Fu;  // [11010101|00000011|00100000|00011111].

  if (ASMJIT_UNLIKELY(!_code))
    return reportError(DebugUtils::errored(kErrorNotInitialized));

  if (ASMJIT_UNLIKELY(uint32_t(alignMode) > uint32_t(AlignMode::kMaxValue)))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment <= 1) return kErrorOk;

  if (ASMJIT_UNLIKELY(alignment > Globals::kMaxAlignment ||
                      !Support::isPowerOf2(alignment)))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  uint32_t i = uint32_t(Support::alignUpDiff<size_t>(offset(), alignment));
  if (i == 0) return kErrorOk;

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, i));

  switch (alignMode) {
    case AlignMode::kCode: {
      uint32_t pattern = kNopA64;

      if (ASMJIT_UNLIKELY(offset() & 0x3u))
        return DebugUtils::errored(kErrorInvalidState);

      while (i >= 4) {
        writer.emit32uLE(pattern);
        i -= 4;
      }

      ASMJIT_ASSERT(i == 0);
      break;
    }

    case AlignMode::kData:
    case AlignMode::kZero:
      writer.emitZeros(i);
      break;
  }

  writer.done(this);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<128> sb;
    sb.appendChars(' ', _logger->indentation(FormatIndentationGroup::kCode));
    sb.appendFormat("align %u\n", alignment);
    _logger->log(sb);
  }
#endif

  return kErrorOk;
}

// a64::Assembler - Events
// =======================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  _instructionAlignment = uint8_t(4);
  assignEmitterFuncs(this);

  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif  // !ASMJIT_NO_LOONGARCH64
