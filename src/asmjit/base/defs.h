// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_DEFS_H
#define _ASMJIT_BASE_DEFS_H

// [Dependencies - AsmJit]
#include "../base/intutil.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BaseAssembler;
struct BaseCompiler;

// ============================================================================
// [asmjit::kOperandType]
// ============================================================================

//! @brief Operand types that can be encoded in @ref Operand.
ASMJIT_ENUM(kOperandType) {
  //! @brief Invalid operand, used only internally (not initialized Operand).
  kOperandTypeNone = 0,
  //! @brief Operand is a register.
  kOperandTypeReg = 1,
  //! @brief Operand is a variable.
  kOperandTypeVar = 2,
  //! @brief Operand is a memory.
  kOperandTypeMem = 3,
  //! @brief Operand is an immediate value.
  kOperandTypeImm = 4,
  //! @brief Operand is a label.
  kOperandTypeLabel = 5
};

// ============================================================================
// [asmjit::kOperandId]
// ============================================================================

//! @brief Operand id masks used to determine the operand type.
ASMJIT_ENUM(kOperandId) {
  //! @brief Operand id refers to @ref BaseVar.
  kOperandIdVar = 0x80000000U,
  //! @brief Operand id to real index mask.
  kOperandIdNum = 0x7FFFFFFFU
};

// ============================================================================
// [asmjit::kRegClass]
// ============================================================================

//! @brief Register class.
ASMJIT_ENUM(kRegClass) {
  //! @brief Gp register class (any architecture).
  kRegClassGp = 0,

  //! @brief Invalid register class.
  kRegClassInvalid = 0xFF
};

// ============================================================================
// [asmjit::kInstCode]
// ============================================================================

//! @brief Instruction codes (stub).
ASMJIT_ENUM(kInstCode) {
  //! @brief No instruction.
  kInstNone = 0
};

// ============================================================================
// [asmjit::kInstOptions]
// ============================================================================

//! @brief Instruction options (stub).
ASMJIT_ENUM(kInstOptions) {
  //! @brief No instruction options.
  kInstOptionNone = 0x00,

  //! @brief Emit short form of the instruction.
  //!
  //! X86/X64:
  //!
  //! Short form is mostly related to jmp and jcc instructions, but can be used
  //! by other instructions supporting 8-bit or 32-bit immediates. This option
  //! can be dangerous if the short jmp/jcc is required, but not encodable due
  //! to large displacement, in such case an error happens and the whole
  //! assembler/compiler stream is unusable.
  kInstOptionShortForm = 0x01,

  //! @brief Emit long form of the instruction.
  //!
  //! X86/X64:
  //!
  //! Long form is mosrlt related to jmp and jcc instructions, but like the
  //! @ref kInstOptionShortForm option it can be used by other instructions
  //! supporting both 8-bit and 32-bit immediates.
  kInstOptionLongForm = 0x02,

  //! @brief Condition is likely to be taken (instruction).
  kInstOptionTaken = 0x04,
  //! @brief Condition is unlikely to be taken (instruction).
  kInstOptionNotTaken = 0x08
};

// ============================================================================
// [asmjit::kSize]
// ============================================================================

//! @brief Common size of registers and pointers.
ASMJIT_ENUM(kSize) {
  //! @brief 1 byte size.
  kSizeByte = 1,
  //! @brief 2 bytes size.
  kSizeWord = 2,
  //! @brief 4 bytes size.
  kSizeDWord = 4,
  //! @brief 8 bytes size.
  kSizeQWord = 8,
  //! @brief 10 bytes size.
  kSizeTWord = 10,
  //! @brief 16 bytes size.
  kSizeOWord = 16,
  //! @brief 32 bytes size.
  kSizeYWord = 32
};

// ============================================================================
// [asmjit::kMemType]
// ============================================================================

//! @brief Type of memory operand.
ASMJIT_ENUM(kMemType) {
  //! @brief Memory operand is a combination of base register, optional index
  //! register and optional displacement.
  //!
  //! @note The @ref Assembler interprets @ref kMemTypeBaseIndex and @ref
  //! kMemTypeStackIndex types the same way, but @ref Compiler interprets
  //! @ref kMemTypeBaseIndex as [base + index] and @ref kMemTypeStackIndex as
  //! [stack(base) + index].
  kMemTypeBaseIndex = 0,

  //! @brief Memory operand is a combination of variable's memory location,
  //! optional index register and displacement.
  //!
  //! @note The @ref Assembler interprets @ref kMemTypeBaseIndex and @ref
  //! kMemTypeStackIndex types in the same way, but @ref Compiler interprets
  //! @ref kMemTypeBaseIndex as [base + index] and @ref kMemTypeStackIndex as
  //! [stack(base) + index].
  kMemTypeStackIndex = 1,

  //! @brief Memory operand refers to the memory location specified by a label.
  kMemTypeLabel = 2,
  //! @brief Memory operand is an absolute memory location.
  //!
  //! Supported mostly by x86, truncated to a 32-bit value when running in
  //! 64-bit mode (x64).
  kMemTypeAbsolute = 3
};

// ============================================================================
// [asmjit::kVarType]
// ============================================================================

ASMJIT_ENUM(kVarType) {
  //! @brief Variable is signed 8-bit integer.
  kVarTypeInt8 = 0,
  //! @brief Variable is unsigned 8-bit integer.
  kVarTypeUInt8 = 1,
  //! @brief Variable is signed 16-bit integer.
  kVarTypeInt16 = 2,
  //! @brief Variable is unsigned 16-bit integer.
  kVarTypeUInt16 = 3,
  //! @brief Variable is signed 32-bit integer.
  kVarTypeInt32 = 4,
  //! @brief Variable is unsigned 32-bit integer.
  kVarTypeUInt32 = 5,
  //! @brief Variable is signed 64-bit integer.
  kVarTypeInt64 = 6,
  //! @brief Variable is unsigned 64-bit integer.
  kVarTypeUInt64 = 7,

  //! @brief Variable is target @c intptr_t (not compatible with host @c intptr_t).
  kVarTypeIntPtr = 8,
  //! @brief Variable is target @c uintptr_t (not compatible with host @c uintptr_t).
  kVarTypeUIntPtr = 9,

  //! @brief Variable is 32-bit floating point (single precision).
  kVarTypeFp32 = 10,
  //! @brief Variable is 64-bit floating point (double precision).
  kVarTypeFp64 = 11,
  //! @brief Variable is 80-bit or 128-bit floating point (extended precision).
  //!
  //! @note Experimental, better not to use.
  kVarTypeFpEx = 12,

  //! @brief Invalid variable type.
  kVarTypeInvalid = 0xFF,

  //! @internal
  _kVarTypeIntStart = kVarTypeInt8,
  //! @internal
  _kVarTypeIntEnd = kVarTypeUIntPtr
};

// ============================================================================
// [asmjit::kRelocMode]
// ============================================================================

ASMJIT_ENUM(kRelocMode) {
  kRelocAbsToAbs = 0,
  kRelocRelToAbs = 1,
  kRelocAbsToRel = 2,
  kRelocTrampoline = 3
};

// ============================================================================
// [asmjit::Ptr]
// ============================================================================

//! @brief 64-bit signed pointer, compatible with JIT and non-JIT generators.
typedef int64_t SignedPtr;

//! @brief 64-bit unsigned pointer, compatible with JIT and non-JIT generators.
typedef uint64_t Ptr;

// ============================================================================
// [asmjit::Operand]
// ============================================================================

//! @brief Operand can contain register, memory location, immediate, or label.
struct Operand {
  // --------------------------------------------------------------------------
  // [Structs]
  // --------------------------------------------------------------------------

  //! @internal
  //!
  //! @brief Base operand data.
  struct BaseOp {
    //! @brief Type of operand, see @c kOperandType.
    uint8_t op;
    //! @brief Size of operand (register, address, immediate, or variable).
    uint8_t size;
    //! @brief Flags, each operand uses this byte for something else.
    uint8_t reserved0;
    //! @brief Reserved (not used).
    uint8_t reserved1;

    //! @brief Operand id (private variable for @ref BaseAssembler and
    //! @ref BaseCompiler classes).
    //!
    //! @note Uninitialized operand has always set id to @ref kInvalidValue.
    uint32_t id;
  };

  //! @internal
  //!
  //! @brief Register or Variable operand data.
  struct VRegOp {
    //! @brief Type of operand, @c kOperandTypeReg.
    uint8_t op;
    //! @brief Size of register or variable.
    uint8_t size;

    union {
      //! @brief Register code = (type << 8) | index.
      uint16_t code;

      //! @brief Register type and index access.
      struct {
#if defined(ASMJIT_HOST_LE)
        //! @brief Register index.
        uint8_t index;
        //! @brief Register type.
        uint8_t type;
#else
        //! @brief Register type.
        uint8_t type;
        //! @brief Register index.
        uint8_t index;
#endif // ASMJIT_HOST
      };
    };

    //! @brief Variable id (used by @ref BaseCompiler to identify variables).
    uint32_t id;

    //! @brief Variable type.
    uint32_t vType;
    //! @internal
    //!
    //! @brief Unused.
    uint32_t vUnused;
  };

  //! @internal
  //!
  //! @brief Memory or Variable operand data.
  struct VMemOp {
    //! @brief Type of operand, @c kOperandTypeMem.
    uint8_t op;
    //! @brief Size of the pointer in bytes.
    uint8_t size;
    //! @brief Type of the memory operand, see @ref kMemType.
    uint8_t type;
    //! X86/X64 layout:
    //!   - segment  [3 bits], see @ref kSeg.
    //!   - shift    [2 bits], index register shift (0 to 3).
    uint8_t flags;

    //! @brief Base register, variable or label id.
    uint32_t base;
    //! @brief Index register or variable.
    uint32_t index;
    //! @brief 32-bit displacement or absolute address.
    int32_t displacement;
  };

  //! @internal
  //!
  //! @brief Immediate operand data.
  struct ImmOp {
    //! @brief Type of operand, @ref kOperandTypeImm.
    uint8_t op;
    //! @brief Size of immediate (or 0 to autodetect).
    uint8_t size;
    //! @brief Reserved (not used).
    uint8_t reserved0;
    //! @brief Reserved (not used).
    uint8_t reserved1;

    //! @brief Operand id (@ref kInvalidValue).
    uint32_t id;

    union {
      //! @brief 8x signed 8-bit immediate values.
      int8_t _i8[8];
      //! @brief 8x unsigned 8-bit immediate values.
      uint8_t _u8[8];

      //! @brief 4x signed 16-bit immediate values.
      int16_t _i16[4];
      //! @brief 4x unsigned 16-bit immediate values.
      uint16_t _u16[4];

      //! @brief 2x signed 32-bit immediate values.
      int32_t _i32[2];
      //! @brief 2x unsigned 32-bit immediate values.
      uint32_t _u32[2];

      //! @brief 1x signed 64-bit immediate value.
      int64_t _i64[1];
      //! @brief 1x unsigned 64-bit immediate value.
      uint64_t _u64[1];

      //! @brief 2x SP-FP values.
      float _f32[2];
      //! @brief 1x DP-FP value.
      double _f64[1];
    } value;
  };

  //! @internal
  //!
  //! @brief Label operand data.
  struct LabelOp {
    //! @brief Type of operand, @c kOperandTypeLabel.
    uint8_t op;
    //! @brief Reserved (not used).
    uint8_t size;
    //! @brief Reserved (not used).
    uint8_t reserved0;
    //! @brief Reserved (not used).
    uint8_t reserved1;

    //! @brief Operand id.
    uint32_t id;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create an uninitialized operand.
  ASMJIT_INLINE Operand() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeNone, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(0, 0);
  }

  //! @brief Create a reference to @a other operand.
  ASMJIT_INLINE Operand(const Operand& other) {
    _init(other);
  }

  explicit ASMJIT_INLINE Operand(const _DontInitialize&) {}

  // --------------------------------------------------------------------------
  // [Operand]
  // --------------------------------------------------------------------------

  //! @brief Clone Operand.
  ASMJIT_INLINE Operand clone() const {
    return Operand(*this);
  }

  // --------------------------------------------------------------------------
  // [Init & Copy]
  // --------------------------------------------------------------------------

  //! @internal
  //!
  //! @brief Initialize operand to @a other (used by constructors).
  ASMJIT_INLINE void _init(const Operand& other) { ::memcpy(this, &other, sizeof(Operand)); }

  ASMJIT_INLINE void _init_packed_op_sz_b0_b1_id(uint32_t op, uint32_t sz, uint32_t r0, uint32_t r1, uint32_t id) {
    // This hack is not for performance, but to decrease the size of the binary
    // generated when constructing AsmJit operands (mostly for third parties).
    // Some compilers are not able to join four BYTE writes to a single DWORD
    // write. Because the 'a', 'b', 'c' and 'd' variables are usually compile
    // time constants the compiler can do a really nice job if they are joined
    // by using bitwise operations.
    _packed[0].setPacked_2x32(IntUtil::pack32_4x8(op, sz, r0, r1), id);
  }

  ASMJIT_INLINE void _init_packed_op_sz_w0_id(uint32_t op, uint32_t sz, uint32_t w0, uint32_t id) {
    _packed[0].setPacked_2x32(IntUtil::pack32_2x8_1x16(op, sz, w0), id);
  }

  ASMJIT_INLINE void _init_packed_d0_d1(uint32_t u0, uint32_t u1) {
    _packed[0].setPacked_2x32(u0, u1);
  }

  ASMJIT_INLINE void _init_packed_d2_d3(uint32_t u2, uint32_t u3) {
    _packed[1].setPacked_2x32(u2, u3);
  }

  //! @internal
  //!
  //! @brief Initialize operand to @a other (used by assign operators).
  ASMJIT_INLINE void _copy(const Operand& other) { ::memcpy(this, &other, sizeof(Operand)); }

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  template<typename T>
  ASMJIT_INLINE T& getData() { return reinterpret_cast<T&>(_base); }

  template<typename T>
  ASMJIT_INLINE const T& getData() const { return reinterpret_cast<const T&>(_base); }

  // --------------------------------------------------------------------------
  // [Type]
  // --------------------------------------------------------------------------

  //! @brief Get type of the operand, see @ref kOperandType.
  ASMJIT_INLINE uint32_t getOp() const { return _base.op; }

  //! @brief Get whether the operand is none (@ref kOperandTypeNone).
  ASMJIT_INLINE bool isNone() const { return (_base.op == kOperandTypeNone); }
  //! @brief Get whether the operand is any register (@ref kOperandTypeReg).
  ASMJIT_INLINE bool isReg() const { return (_base.op == kOperandTypeReg); }
  //! @brief Get whether the operand is variable (@ref kOperandTypeVar).
  ASMJIT_INLINE bool isVar() const { return (_base.op == kOperandTypeVar); }
  //! @brief Get whether the operand is memory address (@ref kOperandTypeMem).
  ASMJIT_INLINE bool isMem() const { return (_base.op == kOperandTypeMem); }
  //! @brief Get whether the operand is an immediate value (@ref kOperandTypeImm).
  ASMJIT_INLINE bool isImm() const { return (_base.op == kOperandTypeImm); }
  //! @brief Get whether the operand is label (@ref kOperandTypeLabel).
  ASMJIT_INLINE bool isLabel() const { return (_base.op == kOperandTypeLabel); }

  // --------------------------------------------------------------------------
  // [Type - Combined]
  // --------------------------------------------------------------------------

  //! @brief Get whether the operand is register of @a type.
  ASMJIT_INLINE bool isRegType(uint32_t type) const {
    return (_packed[0].u32[0] & IntUtil::pack32_2x8_1x16(0xFF, 0, 0xFF00)) == IntUtil::pack32_2x8_1x16(kOperandTypeReg, 0, (type << 8));
  }

  //! @brief Get whether the operand is register and of @a type and @a index.
  ASMJIT_INLINE bool isRegCode(uint32_t type, uint32_t index) const {
    return (_packed[0].u32[0] & IntUtil::pack32_2x8_1x16(0xFF, 0, 0xFFFF)) == IntUtil::pack32_2x8_1x16(kOperandTypeReg, 0, (type << 8) + index);
  }

  //! @brief Get whether the operand is a register or memory.
  ASMJIT_INLINE bool isRegOrMem() const {
    ASMJIT_ASSERT(kOperandTypeReg == 1);
    ASMJIT_ASSERT(kOperandTypeMem == 3);
    return (static_cast<uint32_t>(_base.op) | 0x2U) == 0x3U;
  }

  //! @brief Get whether the operand is variable or memory.
  ASMJIT_INLINE bool isVarOrMem() const {
    ASMJIT_ASSERT(kOperandTypeVar == 2);
    ASMJIT_ASSERT(kOperandTypeMem == 3);
    return (static_cast<uint32_t>(_base.op) - 2U) <= 1;
  }

  // --------------------------------------------------------------------------
  // [Size]
  // --------------------------------------------------------------------------

  //! @brief Get size of the operand in bytes.
  ASMJIT_INLINE uint32_t getSize() const { return _base.size; }

  // --------------------------------------------------------------------------
  // [Id]
  // --------------------------------------------------------------------------

  //! @brief Get operand id (Operand id's are used internally by
  //! @ref BaseAssembler and @ref BaseCompiler classes).
  //!
  //! @note There is no way how to change or remove operand id. If you don't
  //! need the operand just assign different operand to this one.
  ASMJIT_INLINE uint32_t getId() const { return _base.id; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    //! @brief Base data.
    BaseOp _base;
    //! @brief Register or variable data.
    VRegOp _vreg;
    //! @brief Memory data.
    VMemOp _vmem;
    //! @brief Immediate data.
    ImmOp _imm;
    //! @brief Label data.
    LabelOp _label;

    //! @brief Packed operand as two 64-bit integers.
    UInt64 _packed[2];
  };
};

ASMJIT_VAR const Operand noOperand;

// ============================================================================
// [asmjit::OperandUtil]
// ============================================================================

//! @brief Operand utilities.
struct OperandUtil {
  //! @brief Make variable id.
  static ASMJIT_INLINE uint32_t makeVarId(uint32_t id) {
    return id | kOperandIdVar;
  }

  //! @brief Make label id.
  static ASMJIT_INLINE uint32_t makeLabelId(uint32_t id) {
    return id;
  }

  //! @brief Strip variable id bit so it becomes a pure index to VarData[] array.
  static ASMJIT_INLINE uint32_t stripVarId(uint32_t id) {
    return id & 0x7FFFFFFFU;
  }

  //! @brief Get whether the id refers to @ref BaseVar.
  //!
  //! @note The function will never return @c true if the id is @c kInvalidValue.
  //! The trick is to compare a given id to -1 (kInvalidValue) so we check both
  //! using only one comparison.
  static ASMJIT_INLINE bool isVarId(uint32_t id) {
    return static_cast<int32_t>(id) < -1;
  }

  //! @brief Get whether the id refers to @ref Label.
  //!
  //! @note The function will never return @c true if the id is @c kInvalidValue.
  static ASMJIT_INLINE bool isLabelId(uint32_t id) {
    return static_cast<int32_t>(id) >= 0;
  }
};

// ============================================================================
// [asmjit::BaseReg]
// ============================================================================

//! @brief Base class for all register operands.
struct BaseReg : public Operand {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy base register.
  ASMJIT_INLINE BaseReg() : Operand(DontInitialize)
  { _init_packed_op_sz_w0_id(kOperandTypeReg, 0, (kInvalidReg << 8) + kInvalidReg, kInvalidValue); }

  //! @brief Create a new base register.
  ASMJIT_INLINE BaseReg(uint32_t type, uint32_t index, uint32_t size) : Operand(DontInitialize)
  { _init_packed_op_sz_w0_id(kOperandTypeReg, size, (type << 8) + index, kInvalidValue); }

  //! @brief Create a new reference to @a other.
  ASMJIT_INLINE BaseReg(const BaseReg& other) : Operand(other) {}

  explicit ASMJIT_INLINE BaseReg(const _DontInitialize&) : Operand(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [BaseReg Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone BaseReg operand.
  ASMJIT_INLINE BaseReg clone() const {
    return BaseReg(*this);
  }

  //! @brief Get whether register code is equal to @a type.
  ASMJIT_INLINE bool isRegType(uint32_t type) const {
    return _vreg.type == type;
  }

  //! @brief Get whether register code is equal to @a type.
  ASMJIT_INLINE bool isRegCode(uint32_t code) const {
    return _vreg.code == code;
  }

  //! @brief Get whether register code is equal to @a type.
  ASMJIT_INLINE bool isRegCode(uint32_t type, uint32_t index) const {
    return _vreg.code == (type << 8) + index;
  }

  //! @brief Get register code that equals to '(type << 8) + index'.
  ASMJIT_INLINE uint32_t getRegCode() const {
    return _vreg.code;
  }

  //! @brief Get register type.
  ASMJIT_INLINE uint32_t getRegType() const {
    return _vreg.type;
  }

  //! @brief Get register index.
  ASMJIT_INLINE uint32_t getRegIndex() const {
    return _vreg.index;
  }

#define ASMJIT_REG_OP(_Type_) \
  ASMJIT_INLINE _Type_ clone() const { \
    return _Type_(*this); \
  } \
  \
  /*! @brief Set register @a size. */ \
  ASMJIT_INLINE _Type_& setSize(uint32_t size) { \
    _vreg.size = static_cast<uint8_t>(size); \
    return *this; \
  } \
  \
  /*! @brief Set register @a code. */ \
  ASMJIT_INLINE _Type_& setCode(uint32_t code) { \
    _vreg.code = static_cast<uint16_t>(code); \
    return *this; \
  } \
  \
  /*! @brief Set register @a type and @a index. */ \
  ASMJIT_INLINE _Type_& setCode(uint32_t type, uint32_t index) { \
    _vreg.type = static_cast<uint8_t>(type); \
    _vreg.index = static_cast<uint8_t>(index); \
    return *this; \
  } \
  \
  /*! @brief Set register @a type. */ \
  ASMJIT_INLINE _Type_& setType(uint32_t type) { \
    _vreg.type = static_cast<uint8_t>(type); \
    return *this; \
  } \
  \
  /*! @brief Set register @a index. */ \
  ASMJIT_INLINE _Type_& setIndex(uint32_t index) { \
    _vreg.index = static_cast<uint8_t>(index); \
    return *this; \
  } \
  \
  ASMJIT_INLINE _Type_& operator=(const _Type_& other) { _copy(other); return *this; } \
  \
  ASMJIT_INLINE bool operator==(const _Type_& other) const { return _packed[0].u32[0] == other._packed[0].u32[0]; } \
  ASMJIT_INLINE bool operator!=(const _Type_& other) const { return !operator==(other); }
};

// ============================================================================
// [asmjit::BaseMem]
// ============================================================================

//! @brief Base class for all memory operands.
struct BaseMem : public Operand {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseMem() : Operand(DontInitialize) {
    reset();
  }

  ASMJIT_INLINE BaseMem(const BaseMem& other) : Operand(other) {}
  explicit ASMJIT_INLINE BaseMem(const _DontInitialize&) : Operand(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [BaseMem Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone BaseMem operand.
  ASMJIT_INLINE BaseMem clone() const {
    return BaseMem(*this);
  }

  //! @brief Reset BaseMem operand.
  ASMJIT_INLINE void reset() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, 0, kMemTypeBaseIndex, 0, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, 0);
  }

  //! @brief Get the type of the memory operand, see @c kMemType.
  ASMJIT_INLINE uint32_t getMemType() const { return _vmem.type; }
  //! @brief Get whether the type of the memory operand is either @ref
  //! kMemTypeBaseIndex or @ref kMemTypeStackIndex.
  ASMJIT_INLINE bool isBaseIndexType() const { return _vmem.type <= kMemTypeStackIndex; }

  //! @brief Get whether the memory operand has base register.
  ASMJIT_INLINE bool hasBase() const { return _vmem.base != kInvalidValue; }
  //! @brief Get memory operand base id, or @c kInvalidValue.
  ASMJIT_INLINE uint32_t getBase() const { return _vmem.base; }

  //! @brief Set memory operand size.
  ASMJIT_INLINE BaseMem& setSize(uint32_t size) {
    _vmem.size = static_cast<uint8_t>(size);
    return *this;
  }

  //! @brief Get memory operand relative displacement.
  ASMJIT_INLINE int32_t getDisplacement() const
  { return _vmem.displacement; }

  //! @brief Set memory operand relative displacement.
  ASMJIT_INLINE BaseMem& setDisplacement(int32_t disp) {
    _vmem.displacement = disp;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseMem& operator=(const BaseMem& other) {
    _copy(other);
    return *this;
  }

  ASMJIT_INLINE bool operator==(const BaseMem& other) const {
    return (_packed[0] == other._packed[0]) & (_packed[1] == other._packed[1]);
  }

  ASMJIT_INLINE bool operator!=(const BaseMem& other) const { return !(*this == other); }
};

// ============================================================================
// [asmjit::BaseVar]
// ============================================================================

//! @brief Base class for all variables.
struct BaseVar : public Operand {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseVar() : Operand(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeVar, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, kInvalidValue);
  }

  ASMJIT_INLINE BaseVar(const BaseVar& other) : Operand(other) {}

  explicit ASMJIT_INLINE BaseVar(const _DontInitialize&) : Operand(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [BaseVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone BaseVar operand.
  ASMJIT_INLINE BaseVar clone() const {
    return BaseVar(*this);
  }

  ASMJIT_INLINE uint32_t getVarType() const {
    return _vreg.vType;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseVar& operator=(const BaseVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const BaseVar& other) const
  { return _packed[0] == other._packed[0]; }

  ASMJIT_INLINE bool operator!=(const BaseVar& other) const { return !operator==(other); }
};

// ============================================================================
// [asmjit::Imm]
// ============================================================================

//! @brief Immediate operand.
//!
//! Immediate operand is usually part of instruction itself (it's inlined after
//! or before instruction opcode). Immediates can be only signed or unsigned
//! integers.
//!
//! To create immediate operand, use @c imm() and @c imm_u() constructors or
//! constructors provided by @c Immediate class itself.
struct Imm : public Operand {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new immediate value (initial value is 0).
  Imm() : Operand(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeImm, 0, 0, 0, kInvalidValue);
    _imm.value._i64[0] = 0;
  }

  //! @brief Create a new signed immediate value, assigning the value to @a val.
  explicit Imm(int64_t val) : Operand(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeImm, 0, 0, 0, kInvalidValue);
    _imm.value._i64[0] = val;
  }

  //! @brief Create a new immediate value from @a other.
  ASMJIT_INLINE Imm(const Imm& other) : Operand(other) {}

  explicit ASMJIT_INLINE Imm(const _DontInitialize&) : Operand(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [Immediate Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone Imm operand.
  ASMJIT_INLINE Imm clone() const {
    return Imm(*this);
  }

  //! @brief Get whether the immediate can be casted to 8-bit signed integer.
  ASMJIT_INLINE bool isInt8() const { return IntUtil::isInt8(_imm.value._i64[0]); }
  //! @brief Get whether the immediate can be casted to 8-bit unsigned integer.
  ASMJIT_INLINE bool isUInt8() const { return IntUtil::isUInt8(_imm.value._i64[0]); }

  //! @brief Get whether the immediate can be casted to 16-bit signed integer.
  ASMJIT_INLINE bool isInt16() const { return IntUtil::isInt16(_imm.value._i64[0]); }
  //! @brief Get whether the immediate can be casted to 16-bit unsigned integer.
  ASMJIT_INLINE bool isUInt16() const { return IntUtil::isUInt16(_imm.value._i64[0]); }

  //! @brief Get whether the immediate can be casted to 32-bit signed integer.
  ASMJIT_INLINE bool isInt32() const { return IntUtil::isInt32(_imm.value._i64[0]); }
  //! @brief Get whether the immediate can be casted to 32-bit unsigned integer.
  ASMJIT_INLINE bool isUInt32() const { return IntUtil::isUInt32(_imm.value._i64[0]); }

  //! @brief Get immediate value as 8-bit signed integer.
  ASMJIT_INLINE int8_t getInt8() const { return _imm.value._i8[_ASMJIT_HOST_INDEX(8, 0)]; }
  //! @brief Get immediate value as 8-bit unsigned integer.
  ASMJIT_INLINE uint8_t getUInt8() const { return _imm.value._u8[_ASMJIT_HOST_INDEX(8, 0)]; }
  //! @brief Get immediate value as 16-bit signed integer.
  ASMJIT_INLINE int16_t getInt16() const { return _imm.value._i16[_ASMJIT_HOST_INDEX(4, 0)]; }
  //! @brief Get immediate value as 16-bit unsigned integer.
  ASMJIT_INLINE uint16_t getUInt16() const { return _imm.value._u16[_ASMJIT_HOST_INDEX(4, 0)]; }
  //! @brief Get immediate value as 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32() const { return _imm.value._i32[_ASMJIT_HOST_INDEX(2, 0)]; }
  //! @brief Get immediate value as 32-bit unsigned integer.
  ASMJIT_INLINE uint32_t getUInt32() const { return _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)]; }
  //! @brief Get immediate value as 64-bit signed integer.
  ASMJIT_INLINE int64_t getInt64() const { return _imm.value._i64[0]; }
  //! @brief Get immediate value as 64-bit unsigned integer.
  ASMJIT_INLINE uint64_t getUInt64() const { return _imm.value._u64[0]; }

  //! @brief Get immediate value as intptr_t.
  ASMJIT_INLINE intptr_t getIntPtr() const {
    if (sizeof(intptr_t) == sizeof(int64_t))
      return static_cast<intptr_t>(getInt64());
    else
      return static_cast<intptr_t>(getInt32());
  }

  //! @brief Get immediate value as uintptr_t.
  ASMJIT_INLINE uintptr_t getUIntPtr() const {
    if (sizeof(uintptr_t) == sizeof(uint64_t))
      return static_cast<uintptr_t>(getUInt64());
    else
      return static_cast<uintptr_t>(getUInt32());
  }

  //! @brief Get Lo 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32Lo() const { return _imm.value._i32[_ASMJIT_HOST_INDEX(2, 0)]; }
  //! @brief Get Lo 32-bit signed integer.
  ASMJIT_INLINE uint32_t getUInt32Lo() const { return _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)]; }
  //! @brief Get Hi 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32Hi() const { return _imm.value._i32[_ASMJIT_HOST_INDEX(2, 1)]; }
  //! @brief Get Hi 32-bit signed integer.
  ASMJIT_INLINE uint32_t getUInt32Hi() const { return _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)]; }

  //! @brief Set immediate value to 8-bit signed integer @a val.
  ASMJIT_INLINE Imm& setInt8(int8_t val) {
    if (kArchHost64Bit) {
      _imm.value._i64[0] = static_cast<int64_t>(val);
    }
    else {
      int32_t val32 = static_cast<int32_t>(val);
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 0)] = val32;
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 1)] = val32 >> 31;
    }
    return *this;
  }

  //! @brief Set immediate value to 8-bit unsigned integer @a val.
  ASMJIT_INLINE Imm& setUInt8(uint8_t val) {
    if (kArchHost64Bit) {
      _imm.value._u64[0] = static_cast<uint64_t>(val);
    }
    else {
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)] = static_cast<uint32_t>(val);
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    }
    return *this;
  }

  //! @brief Set immediate value to 16-bit signed integer @a val.
  ASMJIT_INLINE Imm& setInt16(int16_t val) {
    if (kArchHost64Bit) {
      _imm.value._i64[0] = static_cast<int64_t>(val);
    }
    else {
      int32_t val32 = static_cast<int32_t>(val);
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 0)] = val32;
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 1)] = val32 >> 31;
    }
    return *this;
  }

  //! @brief Set immediate value to 16-bit unsigned integer @a val.
  ASMJIT_INLINE Imm& setUInt16(uint16_t val) {
    if (kArchHost64Bit) {
      _imm.value._u64[0] = static_cast<uint64_t>(val);
    }
    else {
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)] = static_cast<uint32_t>(val);
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    }
    return *this;
  }

  //! @brief Set immediate value to 32-bit signed integer @a val.
  ASMJIT_INLINE Imm& setInt32(int32_t val) {
    if (kArchHost64Bit) {
      _imm.value._i64[0] = static_cast<int64_t>(val);
    }
    else {
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 0)] = val;
      _imm.value._i32[_ASMJIT_HOST_INDEX(2, 1)] = val >> 31;
    }
    return *this;
  }

  //! @brief Set immediate value to 32-bit unsigned integer @a val.
  ASMJIT_INLINE Imm& setUInt32(uint32_t val) {
    if (kArchHost64Bit) {
      _imm.value._u64[0] = static_cast<uint64_t>(val);
    }
    else {
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)] = val;
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    }
    return *this;
  }

  //! @brief Set immediate value to 64-bit signed integer @a val.
  ASMJIT_INLINE Imm& setInt64(int64_t val) {
    _imm.value._i64[0] = val;
    return *this;
  }

  //! @brief Set immediate value to 64-bit unsigned integer @a val.
  ASMJIT_INLINE Imm& setUInt64(uint64_t val) {
    _imm.value._u64[0] = val;
    return *this;
  }

  //! @brief Set immediate value to intptr_t @a val.
  ASMJIT_INLINE Imm& setIntPtr(intptr_t val) {
    _imm.value._i64[0] = static_cast<int64_t>(val);
    return *this;
  }

  //! @brief Set immediate value to uintptr_t @a val.
  ASMJIT_INLINE Imm& setUIntPtr(uintptr_t val) {
    _imm.value._u64[0] = static_cast<uint64_t>(val);
    return *this;
  }

  //! @brief Set immediate value as unsigned type to @a val.
  ASMJIT_INLINE Imm& setPtr(void* p) { return setIntPtr((intptr_t)p); }

  // --------------------------------------------------------------------------
  // [Float]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Imm& setFloat(float f) {
    _imm.value._f32[_ASMJIT_HOST_INDEX(2, 0)] = f;
    _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    return *this;
  }

  ASMJIT_INLINE Imm& setDouble(double d) {
    _imm.value._f64[0] = d;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Truncate]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Imm& truncateTo8Bits() {
    if (kArchHost64Bit) {
      _imm.value._u64[0] &= static_cast<uint64_t>(0x000000FFU);
    }
    else {
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)] &= 0x000000FFU;
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    }
    return *this;
  }

  ASMJIT_INLINE Imm& truncateTo16Bits() {
    if (kArchHost64Bit) {
      _imm.value._u64[0] &= static_cast<uint64_t>(0x0000FFFFU);
    }
    else {
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 0)] &= 0x0000FFFFU;
      _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    }
    return *this;
  }

  ASMJIT_INLINE Imm& truncateTo32Bits() {
    _imm.value._u32[_ASMJIT_HOST_INDEX(2, 1)] = 0;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  //! @brief Assign @a other to the immediate operand.
  ASMJIT_INLINE Imm& operator=(const Imm& other) { _copy(other); return *this; }
};

//! @brief Create signed immediate value operand.
static ASMJIT_INLINE Imm imm(int64_t val) { return Imm(val); }
//! @brief Create unsigned immediate value operand.
static ASMJIT_INLINE Imm imm_u(uint64_t val) { return Imm(static_cast<int64_t>(val)); }
//! @brief Create void* pointer immediate value operand.
static ASMJIT_INLINE Imm imm_ptr(void* p) { return Imm(static_cast<int64_t>((intptr_t)p)); }

// ============================================================================
// [asmjit::Label]
// ============================================================================

//! @brief Label (jump target or data location).
//!
//! Label represents locations typically used as jump targets, but may be also
//! used as position where are stored constants or static variables. If you
//! want to use @c Label you need first to associate it with @ref BaseAssembler
//! or @ref BaseCompiler instance. To create new label use @ref
//! BaseAssembler::newLabel() or @ref BaseCompiler::newLabel().
//!
//! Example of using labels:
//!
//! @code
//! // Create Assembler/Compiler.
//! Assembler a;
//!
//! // Create Label instance.
//! Label L_1(a);
//!
//! // ... your code ...
//!
//! // Using label, see @ref asmjit::BaseAssembler or @ref asmjit::BaseCompiler.
//! a.jump(L_1);
//!
//! // ... your code ...
//!
//! // Bind label to current position, see @ref asmjit::BaseAssembler::bind()
//! // or @ref asmjit::BaseCompiler::bind().
//! a.bind(L_1);
//! @endcode
struct Label : public Operand {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new, unassociated label.
  ASMJIT_INLINE Label() : Operand(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeLabel, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(0, 0);
  }

  explicit ASMJIT_INLINE Label(uint32_t id) : Operand(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeLabel, 0, 0, 0, id);
    _init_packed_d2_d3(0, 0);
  }

  //! @brief Create new initialized label.
  explicit ASMJIT_INLINE Label(BaseAssembler& a);
  //! @brief Create new initialized label.
  explicit ASMJIT_INLINE Label(BaseCompiler& c);

  //! @brief Create reference to another label.
  ASMJIT_INLINE Label(const Label& other) : Operand(other) {}

  explicit ASMJIT_INLINE Label(const _DontInitialize&) : Operand(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Label& operator=(const Label& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const Label& other) const { return _base.id == other._base.id; }
  ASMJIT_INLINE bool operator!=(const Label& other) const { return _base.id != other._base.id; }
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_DEFS_H
