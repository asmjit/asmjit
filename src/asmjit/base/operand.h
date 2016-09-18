// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_OPERAND_H
#define _ASMJIT_BASE_OPERAND_H

// [Dependencies]
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::Operand_]
// ============================================================================

//! Constructor-less \ref Operand.
//!
//! Contains no initialization code and can be used safely to define an array
//! of operands that won't be initialized. This is a \ref Operand compatible
//! data structure designed to be statically initialized or `static const`.
struct Operand_ {
  // --------------------------------------------------------------------------
  // [Operand Type]
  // --------------------------------------------------------------------------

  //! Operand types that can be encoded in \ref Operand.
  ASMJIT_ENUM(OpType) {
    kOpNone         = 0,                 //!< Not an operand or not initialized.
    kOpReg          = 1,                 //!< Operand is a register.
    kOpMem          = 2,                 //!< Operand is a memory.
    kOpImm          = 3,                 //!< Operand is an immediate value.
    kOpLabel        = 4                  //!< Operand is a label.
  };

  // --------------------------------------------------------------------------
  // [Operand Id]
  // --------------------------------------------------------------------------

  //! Operand id helpers useful for id <-> index translation.
  ASMJIT_ENUM(PackedId) {
    kPackedIdMin    = 0x00000100U,                //!< Minimum valid packed-id.
    kPackedIdMax    = 0xFFFFFFFEU,                //!< Maximum valid packed-id.
    kPackedIdCount  = kPackedIdMax - kPackedIdMin //!< Count of valid packed-ids.
  };

  // --------------------------------------------------------------------------
  // [Operand Utilities]
  // --------------------------------------------------------------------------

  //! Get if the given `id` is a valid packed-id that can be used by Operand.
  //! Packed ids are those equal or greater than `kPackedIdMin` and lesser or
  //! equal to `kPackedIdMax`. This concept was created to support virtual
  //! registers and to make them distinguishable from physical ones. It allows
  //! a single uint32_t to contain either physical register id or virtual
  //! register id represented as `packed-id`. This concept is used also for
  //! labels to make the API consistent.
  static ASMJIT_INLINE bool isPackedId(uint32_t id) noexcept { return id - kPackedIdMin <= kPackedIdCount; }
  //! Convert a real-id into a packed-id that can be stored in Operand.
  static ASMJIT_INLINE uint32_t packId(uint32_t id) noexcept { return id + kPackedIdMin; }
  //! Convert a packed-id back to real-id.
  static ASMJIT_INLINE uint32_t unpackId(uint32_t id) noexcept { return id - kPackedIdMin; }

  static ASMJIT_INLINE uint32_t makeSignature(uint32_t opType, uint32_t subType, uint32_t payload, uint32_t size) noexcept {
    return Utils::pack32_4x8(opType, subType, payload, size);
  }

  static ASMJIT_INLINE uint32_t makeRegSignature(uint32_t regType, uint32_t regKind, uint32_t size) noexcept {
    return Utils::pack32_4x8(kOpReg, regType, regKind, size);
  }

  // --------------------------------------------------------------------------
  // [Operand Data]
  // --------------------------------------------------------------------------

  //! Any operand.
  struct AnyData {
    uint8_t op;                          //!< Type of the operand (see \ref OpType).
    uint8_t subType;                     //!< Subtype - depends on `op`.
    uint8_t payload;                     //!< Payload - depends on `op`.
    uint8_t size;                        //!< Size of the operand (register, address, immediate).
    uint32_t id;                         //!< Operand id or `kInvalidValue`.
    uint32_t reserved8_4;                //!< \internal
    uint32_t reserved12_4;               //!< \internal
  };

  //! Register operand data.
  struct RegData {
    uint8_t op;                          //!< Type of the operand (always \ref kOpReg).
    uint8_t regType;                     //!< Register type (also operand sub-type).
    uint8_t regKind;                     //!< Register kind.
    uint8_t size;                        //!< Size of the register.
    uint32_t id;                         //!< Physical or virtual register id.
    uint32_t reserved8_4;                //!< \internal
    uint32_t reserved12_4;               //!< \internal
  };

  //! Memory operand data.
  struct MemData {
    uint8_t op;                          //!< Type of the operand (always \ref kOpMem).
    uint8_t baseIndexType;               //!< Type of BASE and INDEX registers.
    uint8_t flags;                       //!< Architecture dependent flags.
    uint8_t size;                        //!< Size of the memory operand.
    uint32_t index;                      //!< INDEX register id or `kInvalidValue`.

    // [BASE + OFF32] vs just [OFF64].
    union {
      uint64_t offset64;                 //!< 64-bit offset, combining low and high 32-bit parts.
      struct {
#if ASMJIT_ARCH_LE
        uint32_t offsetLo32;             //!< 32-bit low offset part.
        uint32_t base;                   //!< 32-bit high offset part or BASE.
#else
        uint32_t base;                   //!< 32-bit high offset part or BASE.
        uint32_t offsetLo32;             //!< 32-bit low offset part.
#endif
      };
    };
  };

  //! Immediate operand data.
  struct ImmData {
    uint8_t op;                          //!< Type of the operand (always \ref kOpImm).
    uint8_t reserved_1_1;                //!< Must be zero.
    uint8_t reserved_2_1;                //!< Must be zero.
    uint8_t size;                        //!< Size of the immediate (or 0 to autodetect).
    uint32_t id;                         //!< Immediate id (always `kInvalidValue`).
    UInt64 value;                        //!< Immediate value.
  };

  //! Label operand data.
  struct LabelData {
    uint8_t op;                          //!< Type of the operand (always \ref kOpLabel).
    uint8_t reserved_1_1;                //!< Must be zero.
    uint8_t reserved_2_1;                //!< Must be zero.
    uint8_t size;                        //!< Must be zero.
    uint32_t id;                         //!< Label id (`kInvalidValue` if not initialized).
    uint32_t reserved8_4;                //!< \internal
    uint32_t reserved12_4;               //!< \internal
  };

  // --------------------------------------------------------------------------
  // [Init & Copy]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Initialize the operand to `other` (used by constructors).
  ASMJIT_INLINE void _init(const Operand_& other) noexcept { ::memcpy(this, &other, sizeof(Operand_)); }

  //! \internal
  ASMJIT_INLINE void _initReg(uint32_t signature, uint32_t id) {
    _init_packed_d0_d1(signature, id);
    _init_packed_d2_d3(0, 0);
  }

  //! \internal
  ASMJIT_INLINE void _init_packed_op_b1_b2_sz_id(uint32_t op, uint32_t r0, uint32_t r1, uint32_t size, uint32_t id) noexcept {
    // This hack is not for performance, it's to decrease the size of the binary
    // generated when constructing AsmJit operands (mostly for third parties).
    // Some compilers are not able to join four BYTE writes to a single DWORD
    // write. Because the `op`, `r0`, `r1`, and `size` arguments are usually compile
    // time constants the compiler can do a really nice job if they are joined
    // by using bitwise operations.
    _packed[0].setPacked_2x32(makeSignature(op, r0, r1, size), id);
  }

  //! \internal
  ASMJIT_INLINE void _init_packed_d0_d1(uint32_t d0, uint32_t d1) noexcept { _packed[0].setPacked_2x32(d0, d1); }
  //! \internal
  ASMJIT_INLINE void _init_packed_d2_d3(uint32_t d2, uint32_t d3) noexcept { _packed[1].setPacked_2x32(d2, d3); }

  //! \internal
  //!
  //! Initialize the operand from `other` (used by operator overloads).
  ASMJIT_INLINE void copyFrom(const Operand_& other) noexcept { ::memcpy(this, &other, sizeof(Operand_)); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get if the operand matches the given signature `sign`.
  ASMJIT_INLINE bool hasSignature(uint32_t signature) const noexcept { return _signature == signature; }

  //! Get if the operand matches a signature composed of `opType`, `subType`, `payload`, and `size`,
  ASMJIT_INLINE bool hasSignature(uint32_t opType, uint32_t subType, uint32_t payload, uint32_t size) const noexcept {
    return _signature == makeSignature(opType, subType, payload, size);
  }
  //! Get if the operand matches a signature of the `other` operand.
  ASMJIT_INLINE bool hasSignature(const Operand_& other) const noexcept {
    return _signature == other.getSignature();
  }

  //! Get a 32-bit operand signature.
  //!
  //! Signature is first 4 bytes of the operand data. It's used mostly for
  //! operand checking as it's much faster to check 4 bytes at once than having
  //! to check these bytes individually.
  ASMJIT_INLINE uint32_t getSignature() const noexcept { return _signature; }
  //! Set the operand signature (see \ref getSignature).
  //!
  //! Improper use of `setSignature()` can lead to hard-to-debug errors.
  ASMJIT_INLINE void setSignature(uint32_t signature) noexcept { _signature = signature; }
  //! \overload
  ASMJIT_INLINE void setSignature(uint32_t opType, uint32_t subType, uint32_t payload, uint32_t size) noexcept {
    _signature = makeSignature(opType, subType, payload, size);
  }

  //! Get type of the operand, see \ref OpType.
  ASMJIT_INLINE uint32_t getOp() const noexcept { return _any.op; }
  //! Get if the operand is none (\ref kOpNone).
  ASMJIT_INLINE bool isNone() const noexcept { return _any.op == kOpNone; }
  //! Get if the operand is a register (\ref kOpReg).
  ASMJIT_INLINE bool isReg() const noexcept { return _any.op == kOpReg; }
  //! Get if the operand is a memory location (\ref kOpMem).
  ASMJIT_INLINE bool isMem() const noexcept { return _any.op == kOpMem; }
  //! Get if the operand is an immediate (\ref kOpImm).
  ASMJIT_INLINE bool isImm() const noexcept { return _any.op == kOpImm; }
  //! Get if the operand is a label (\ref kOpLabel).
  ASMJIT_INLINE bool isLabel() const noexcept { return _any.op == kOpLabel; }

  //! Get if the operand is a physical register.
  ASMJIT_INLINE bool isPhysReg() const noexcept { return isReg() && _reg.id < kInvalidReg; }
  //! Get if the operand is a virtual register.
  ASMJIT_INLINE bool isVirtReg() const noexcept { return isReg() && isPackedId(_reg.id); }

  //! Get if the operand specifies a size (i.e. the size is not zero).
  ASMJIT_INLINE bool hasSize() const noexcept { return _any.size != 0; }
  //! Get if the size of the operand matches `size`.
  ASMJIT_INLINE bool hasSize(uint32_t size) const noexcept { return _any.size == size; }

  //! Get size of the operand (in bytes).
  //!
  //! The value returned depends on the operand type:
  //!   * None  - Should always return zero size.
  //!   * Reg   - Should always return the size of the register. If the register
  //!             size depends on architecture (like \ref X86CReg and \ref X86DReg)
  //!             the size returned should be the greatest possible (so it should
  //!             return 64-bit size in such case).
  //!   * Mem   - Size is optional and will be in most cases zero.
  //!   * Imm   - Should always return zero size.
  //!   * Label - Should always return zero size.
  ASMJIT_INLINE uint32_t getSize() const noexcept { return _any.size; }

  //! Get the operand id.
  //!
  //! The value returned should be interpreted accordingly to the operand type:
  //!   * None  - Should be `kInvalidValue`.
  //!   * Reg   - Physical or virtual register id.
  //!   * Mem   - Multiple meanings - BASE address (register or label id), or
  //!             high value of a 64-bit absolute address.
  //!   * Imm   - Should be `kInvalidValue`.
  //!   * Label - Label id if it was created by using `newLabel()` or `kInvalidValue`
  //!             if the label is invalid or uninitialized.
  ASMJIT_INLINE uint32_t getId() const noexcept { return _any.id; }

  //! Get if the operand is 100% equal to `other`.
  ASMJIT_INLINE bool isEqual(const Operand_& other) const noexcept {
    return (_packed[0] == other._packed[0]) &
           (_packed[1] == other._packed[1]) ;
  }

  //! Get if the operand is a register matching `regType`.
  ASMJIT_INLINE bool isReg(uint32_t regType) const noexcept {
    const uint32_t kMsk = makeSignature(0xFF  , 0xFF   , 0, 0);
    const uint32_t kSgn = makeSignature(kOpReg, regType, 0, 0);
    return (_signature & kMsk) == kSgn;
  }

  //! Get whether the operand is register and of `type` and `id`.
  ASMJIT_INLINE bool isReg(uint32_t regType, uint32_t id) const noexcept {
    return isReg(regType) && getId() == id;
  }

  //! Get whether the operand is a register or memory.
  ASMJIT_INLINE bool isRegOrMem() const noexcept {
    ASMJIT_ASSERT(kOpMem - kOpReg == 1);
    return _any.op >= kOpReg && _any.op <= kOpMem;
  }

  //! Cast this operand to `T` type.
  template<typename T>
  ASMJIT_INLINE T& as() noexcept { return static_cast<T&>(*this); }
  //! Cast this operand to `T` type (const).
  template<typename T>
  ASMJIT_INLINE const T& as() const noexcept { return static_cast<const T&>(*this); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the `Operand` to none.
  //!
  //! None operand is defined the following way:
  //!   - Its signature is zero (kOpNone, and the rest zero as well).
  //!   - Its id is `kInvalidValue`.
  //!   - The reserved8_4 field is set to `kInvalidValue`.
  //!   - The reserved12_4 field is set to zero.
  //!
  //! Reset operand must match the Operand state right after its construction:
  //!
  //! ```
  //! using namespace asmjit;
  //!
  //! Operand a;
  //! Operand b;
  //! assert(a == b);
  //!
  //! b = x86::eax;
  //! assert(a != b);
  //!
  //! b.reset();
  //! assert(a == b);
  //! ```
  ASMJIT_INLINE void reset() noexcept {
    _init_packed_op_b1_b2_sz_id(kOpNone, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(0, 0);
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  template<typename T>
  ASMJIT_INLINE bool operator==(const T& other) const noexcept { return  isEqual(other); }
  template<typename T>
  ASMJIT_INLINE bool operator!=(const T& other) const noexcept { return !isEqual(other); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    AnyData _any;                        //!< Generic data.
    RegData _reg;                        //!< Physical or virtual register data.
    MemData _mem;                        //!< Memory address data.
    ImmData _imm;                        //!< Immediate value data.
    LabelData _label;                    //!< Label data.

    uint32_t _signature;                 //!< Operand signature (first 32-bits).
    UInt64 _packed[2];                   //!< Operand packed into two 64-bit integers.
  };
};

// ============================================================================
// [asmjit::Operand]
// ============================================================================

//! Operand can contain register, memory location, immediate, or label.
class Operand : public Operand_ {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create an uninitialized operand.
  ASMJIT_INLINE Operand() noexcept { reset(); }
  //! Create a reference to `other` operand.
  ASMJIT_INLINE Operand(const Operand& other) noexcept { _init(other); }
  //! Create a reference to `other` operand.
  explicit ASMJIT_INLINE Operand(const Operand_& other) noexcept { _init(other); }
  //! Create a completely uninitialized operand (dangerous).
  explicit ASMJIT_INLINE Operand(const _NoInit&) noexcept {}

  // --------------------------------------------------------------------------
  // [Clone]
  // --------------------------------------------------------------------------

  //! Clone the `Operand`.
  ASMJIT_INLINE Operand clone() const noexcept { return Operand(*this); }

  ASMJIT_INLINE Operand& operator=(const Operand_& other) noexcept { copyFrom(other); return *this; }
};

// ============================================================================
// [asmjit::Label]
// ============================================================================

//! Label (jump target or data location).
//!
//! Label represents a location in code typically used as a jump target, but
//! may be also a reference to some data or a static variable. Label has to be
//! explicitly created by CodeEmitter.
//!
//! Example of using labels:
//!
//! ~~~
//! // Create a CodeEmitter (for example X86Assembler).
//! X86Assembler a;
//!
//! // Create Label instance.
//! Label L1 = a.newLabel();
//!
//! // ... your code ...
//!
//! // Using label.
//! a.jump(L1);
//!
//! // ... your code ...
//!
//! // Bind label to the current position, see `CodeEmitter::bind()`.
//! a.bind(L1);
//! ~~~
class Label : public Operand {
public:
  enum {
    kMaxNameLength = 2048                //!< Maximum length of label/symbol name.
  };

  //! Type of the Label.
  enum Type {
    kTypeAnonymous = 0,                  //!< Anonymous (unnamed) label.
    kTypeLocal     = 1,                  //!< Local label (always has parentId).
    kTypeGlobal    = 2,                  //!< Global label (never has parentId).
    kTypeCount     = 3                   //!< Number of label types.
  };

  // TODO: Find a better place, find a better name.
  enum {
    //! Label tag is used as a sub-type, forming a unique signature across all
    //! operand types as 0x1 is never associated with any register (reg-type).
    //! This means that a memory operand's BASE register can be constructed
    //! from virtually any operand (register vs. label) by just assigning its
    //! type (reg type or label-tag) and operand id.
    kLabelTag = 0x1
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create new, unassociated label.
  ASMJIT_INLINE Label() noexcept : Operand(NoInit) { reset(); }
  //! Create a reference to another label.
  ASMJIT_INLINE Label(const Label& other) noexcept : Operand(other) {}

  explicit ASMJIT_INLINE Label(uint32_t id) noexcept : Operand(NoInit) {
    _init_packed_op_b1_b2_sz_id(kOpLabel, 0, 0, 0, id);
    _init_packed_d2_d3(0, 0);
  }

  explicit ASMJIT_INLINE Label(const _NoInit&) noexcept : Operand(NoInit) {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  // TODO: I think that if operand is reset it shouldn't say it's a Label, it
  // should be none like all other operands.
  ASMJIT_INLINE void reset() noexcept {
    _init_packed_op_b1_b2_sz_id(kOpLabel, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(0, 0);
  }

  // --------------------------------------------------------------------------
  // [Label Specific]
  // --------------------------------------------------------------------------

  //! Get if the label was created by CodeEmitter and has an assigned id.
  ASMJIT_INLINE bool isValid() const noexcept { return _label.id != kInvalidValue; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Label& operator=(const Label& other) noexcept { copyFrom(other); return *this; }
};

// ============================================================================
// [asmjit::RegTraits]
// ============================================================================

//! Allows to resolve a signature of `RegT` register at compile-time.
//!
//! Must be provided by an architecture-specific implementation, ambiguous
//! registers like `Reg`, `X86Gp` and `X86Vec` are not resolved by design.
template<typename RegT>
struct RegTraits {};

// ============================================================================
// [asmjit::RegInfo]
// ============================================================================

//! Structure that contains information about a physical register.
//!
//! This information is stored in first 4 bytes of `Reg` operand. It consists
//! of:
//!   * `op`      - Operand type, always \ref Operand::kOpReg.
//!   * `regType` - Register type - platform specific, see \ref X86Reg::Type.
//!   * `regKind` - Register kind - platform specific, see \ref X86Reg::Kind.
//!   * `size`    - Size of the register.
union RegInfo {
  struct {
    uint8_t op;                        //!< Type of the operand (always \ref kOpReg).
    uint8_t regType;                   //!< Register type.
    uint8_t regKind;                   //!< Register kind.
    uint8_t size;                      //!< Size of the register.
  };
  uint32_t signature;
};


// ============================================================================
// [asmjit::Reg]
// ============================================================================

//! Physical/Virtual register operand.
class Reg : public Operand {
public:
  ASMJIT_ENUM(RegType) {
    kRegNone      = 0,                   //!< No register - unused, invalid, multiple meanings.
    _kRegStart    = 2,                   //!< Start of register types (must be honored).
    kRegRip       = _kRegStart,          //!< Universal id of RIP register (if supported).
    kRegMax       = 31                   //!< Maximum possible register id of all architectures.
  };

  //! Register kind.
  ASMJIT_ENUM(Kind) {
    kKindGp = 0                          //!< GP register kind, compatible with all architectures.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy register operand.
  ASMJIT_INLINE Reg() noexcept : Operand() {}
  //! Create a new register operand which is the same as `other` .
  ASMJIT_INLINE Reg(const Reg& other) noexcept : Operand(other) {}
  //! Create a new register operand compatible with `other`, but with a different `id`.
  ASMJIT_INLINE Reg(const Reg& other, uint32_t id) noexcept : Operand(NoInit) {
    _signature = other._signature;
    _reg.id = id;
    _packed[1] = other._packed[1];
  }

  //! Create a register initialized to `signature` and `id`.
  ASMJIT_INLINE Reg(const _Init&, uint32_t signature, uint32_t id) noexcept : Operand(NoInit) { _initReg(signature, id); }
  explicit ASMJIT_INLINE Reg(const _NoInit&) noexcept : Operand(NoInit) {}

  //! Create a new register based on `signature` and `id`.
  static ASMJIT_INLINE Reg fromSignature(uint32_t signature, uint32_t id) noexcept { return Reg(Init, signature, id); }

  // --------------------------------------------------------------------------
  // [Reg Specific]
  // --------------------------------------------------------------------------

  //! Get if the register is valid (either virtual or physical).
  ASMJIT_INLINE bool isValid() const noexcept { return _reg.id != kInvalidValue; }
  //! Get if this is a physical register.
  ASMJIT_INLINE bool isPhysReg() const noexcept { return _reg.id < kInvalidReg; }
  //! Get if this is a virtual register (used by \ref CodeCompiler).
  ASMJIT_INLINE bool isVirtReg() const noexcept { return isPackedId(_reg.id); }

  using Operand_::isReg;

  //! Get if the register type matches `type`.
  ASMJIT_INLINE bool isReg(uint32_t regType) const noexcept { return _reg.regType == regType; }
  //! Get if the register type matches `type` and register id matches `id`.
  ASMJIT_INLINE bool isReg(uint32_t regType, uint32_t id) const noexcept { return isReg(regType) && getId() == id; }

  //! Get if this register is the same as `other`.
  //!
  //! This is just an optimization. Registers by default only use the first
  //! 8 bytes of the Operand, so this method takes advantage of this knowledge
  //! and only compares these 8 bytes. If both operands were created correctly
  //! then `isEqual()` and `isSame()` should give the same answer, however, if
  //! some operands contains a garbage or other metadata in the upper 8 bytes
  //! then `isSame()` may return `true` in cases where `isEqual()` returns
  //! false. However. no such case is known at the moment.
  ASMJIT_INLINE bool isSame(const Reg& other) const noexcept { return _packed[0] == _packed[1]; }

  //! Set a 32-bit operand signature based on traits of `T`.
  template<typename T>
  ASMJIT_INLINE void setSignatureT() noexcept { _signature = RegTraits<T>::kSignature; }

  //! Get the register type.
  ASMJIT_INLINE uint32_t getRegType() const noexcept { return _reg.regType; }
  //! Get the register kind.
  ASMJIT_INLINE uint32_t getRegKind() const noexcept { return _reg.regKind; }

  //! Clone the register operand.
  ASMJIT_INLINE Reg clone() const noexcept { return Reg(*this); }

  //! Cast this register to `RegT` by also changing its signature.
  //!
  //! NOTE: Improper use of `cloneAs()` can lead to hard-to-debug errors.
  template<typename RegT>
  ASMJIT_INLINE RegT cloneAs() const noexcept { return RegT(Init, RegTraits<RegT>::kSignature, getId()); }

  //! Cast this register to `other` by also changing its signature.
  //!
  //! NOTE: Improper use of `cloneAs()` can lead to hard-to-debug errors.
  template<typename RegT>
  ASMJIT_INLINE RegT cloneAs(const RegT& other) const noexcept { return RegT(Init, other.getSignature(), getId()); }

  //! Set the register id to `id`.
  ASMJIT_INLINE void setId(uint32_t id) noexcept { _reg.id = id; }

  //! Set register's `signature` and `id`.
  ASMJIT_INLINE void setSignatureAndId(uint32_t signature, uint32_t id) noexcept {
    _signature = signature;
    _reg.id = id;
  }

#define ASMJIT_DEFINE_ABSTRACT_REG(REG, BASE_REG)                              \
public:                                                                        \
  /*! Default constructor doesn't setup anything, it's like `Operand()`. */    \
  ASMJIT_INLINE REG() ASMJIT_NOEXCEPT                                          \
    : BASE_REG() {}                                                            \
                                                                               \
  /*! Copy the `other` REG register operand. */                                \
  ASMJIT_INLINE REG(const REG& other) ASMJIT_NOEXCEPT                          \
    : BASE_REG(other) {}                                                       \
                                                                               \
  /*! Copy the `other` REG register operand having its id set to `id` */       \
  ASMJIT_INLINE REG(const Reg& other, uint32_t id) ASMJIT_NOEXCEPT             \
    : BASE_REG(other, id) {}                                                   \
                                                                               \
  /*! Create a REG register operand based on `signature` and `id`. */          \
  ASMJIT_INLINE REG(const _Init& init, uint32_t signature, uint32_t id) ASMJIT_NOEXCEPT \
    : BASE_REG(init, signature, id) {}                                         \
                                                                               \
  /*! Create a completely uninitialized REG register operand (garbage). */     \
  explicit ASMJIT_INLINE REG(const _NoInit&) ASMJIT_NOEXCEPT                   \
    : BASE_REG(NoInit) {}                                                      \
                                                                               \
  /*! Clone the register operand. */                                           \
  ASMJIT_INLINE REG clone() const ASMJIT_NOEXCEPT { return REG(*this); }       \
                                                                               \
  /*! Create a new register from register type and id. */                      \
  static ASMJIT_INLINE REG fromTypeAndId(uint32_t regType, uint32_t id) ASMJIT_NOEXCEPT { \
    return REG(Init, signatureOf(regType), id);                                \
  }                                                                            \
                                                                               \
  /*! Create a new register from signature and id. */                          \
  static ASMJIT_INLINE REG fromSignature(uint32_t signature, uint32_t id) ASMJIT_NOEXCEPT { \
    return REG(Init, signature, id);                                           \
  }                                                                            \
                                                                               \
  ASMJIT_INLINE REG& operator=(const REG& other) ASMJIT_NOEXCEPT { copyFrom(other); return *this; } \

#define ASMJIT_DEFINE_FINAL_REG(REG, BASE_REG, TRAITS)                         \
  ASMJIT_DEFINE_ABSTRACT_REG(REG, BASE_REG)                                    \
                                                                               \
  /*! Create a REG register with `id`. */                                      \
  explicit ASMJIT_INLINE REG(uint32_t id) ASMJIT_NOEXCEPT                      \
    : BASE_REG(Init, kSignature, id) {}                                        \
                                                                               \
  enum {                                                                       \
    kThisType  = TRAITS::kType,                                                \
    kThisKind  = TRAITS::kKind,                                                \
    kThisSize  = TRAITS::kSize,                                                \
    kSignature = TRAITS::kSignature                                            \
  };
};

// ============================================================================
// [asmjit::Mem]
// ============================================================================

//! Base class for all memory operands.
//!
//! NOTE: It's tricky to pack all possible cases that define a memory operand
//! into just 16 bytes. The `Mem` splits data into the following parts:
//!
//!   BASE - Base register or label - requires 36 bits total. 4 bits are used
//!     to encode the type of the BASE operand (label vs. register type) and
//!     the remaining 32 bits define the BASE id, which can be a physical or
//!     virtual register index. If BASE type is zero, which is never used as
//!     a register-type and label doesn't use it as well then BASE field
//!     contains a high DWORD of a possible 64-bit absolute address, which is
//!     possible on X64.
//!
//!   INDEX - Index register (or theoretically Label, which doesn't make sense).
//!     Encoding is similar to BASE - it also requires 36 bits and splits the
//!     encoding to INDEX type (4 bits defining the register type) and id (32-bits).
//!
//!   OFFSET - A relative offset of the address. Basically if BASE is specified
//!     the relative displacement adjusts BASE and an optional INDEX. if BASE is
//!     not specified then the OFFSET should be considered as ABSOLUTE address
//!     (at least on X86/X64). In that case its low 32 bits are stored in
//!     DISPLACEMENT field and the remaining high 32 bits are stored in BASE.
//!
//!   OTHER FIELDS - There is rest 8 bits that can be used for whatever purpose.
//!          The X86Mem operand uses these bits to store segment override
//!          prefix and index shift (scale).
class Mem : public Operand {
public:
  //! Memory operand flags.
  ASMJIT_ENUM(Flags) {
    //! Should be encoded as an absolute memory operand.
    kFlagAbs = 0x20,

    //! This memory operand represents a function argument's stack location.
    //!
    //! This flag is used exclusively by \ref CodeCompiler.
    kFlagArgHome = 0x40,

    //! This memory operand represents a virtual register's home-slot.
    //!
    //! This flag is used exclusively by \ref CodeCompiler.
    kFlagRegHome = 0x80
  };

  //! Helper constants to pack BASE and INDEX register types into `MemData::baseIndexType`.
  ASMJIT_ENUM(BaseIndexType) {
    kMemBaseTypeShift  = 0,
    kMemIndexTypeShift = 4,

    kMemBaseTypeMask   = 0xF << kMemBaseTypeShift,
    kMemIndexTypeMask  = 0xF << kMemIndexTypeShift
  };

  static ASMJIT_INLINE uint32_t encodeBaseIndex(uint32_t baseType, uint32_t indexType) noexcept {
    return (baseType << kMemBaseTypeShift) | (indexType << kMemIndexTypeShift);
  }

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Construct a default `Mem` operand, that points to [0].
  ASMJIT_INLINE Mem() noexcept : Operand(NoInit) { reset(); }
  ASMJIT_INLINE Mem(const Mem& other) noexcept : Operand(other) {}

  ASMJIT_INLINE Mem(const _Init&,
    uint32_t baseType, uint32_t baseId,
    uint32_t indexType, uint32_t indexId,
    int32_t off, uint32_t size, uint32_t flags) noexcept : Operand(NoInit) {

    uint32_t baseIndexType = encodeBaseIndex(baseType, indexType);
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndexType, flags, size, indexId);
    _mem.base = baseId;
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }
  explicit ASMJIT_INLINE Mem(const _NoInit&) noexcept : Operand(NoInit) {}

  // --------------------------------------------------------------------------
  // [Mem Internal]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t _unpackFromFlags(uint32_t shift, uint32_t bits) const noexcept {
    return (static_cast<uint32_t>(_mem.flags) >> shift) & bits;
  }

  ASMJIT_INLINE void _packToFlags(uint32_t value, uint32_t shift, uint32_t bits) noexcept {
    ASMJIT_ASSERT(value <= bits);
    _mem.flags = static_cast<uint8_t>(
      (static_cast<uint32_t>(_mem.flags) & ~(bits << shift)) | (value << shift));
  }

  // --------------------------------------------------------------------------
  // [Mem Specific]
  // --------------------------------------------------------------------------

  //! Clone `Mem` operand.
  ASMJIT_INLINE Mem clone() const noexcept { return Mem(*this); }

  //! Reset the memory operand - after reset the memory points to [0].
  ASMJIT_INLINE void reset() noexcept {
    _init_packed_op_b1_b2_sz_id(kOpMem, 0, 0, 0, kInvalidValue);
    _init_packed_d2_d3(0, 0);
  }

  ASMJIT_INLINE uint32_t getFlags() const noexcept { return _mem.flags; }
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (_mem.flags & flag) != 0; }
  ASMJIT_INLINE void addFlags(uint32_t flags) noexcept { _mem.flags |= static_cast<uint8_t>(flags); }
  ASMJIT_INLINE void clearFlags(uint32_t flags) noexcept { _mem.flags &= ~static_cast<uint8_t>(flags); }

  ASMJIT_INLINE bool isAbs() const noexcept { return hasFlag(kFlagAbs); }
  ASMJIT_INLINE bool isArgHome() const noexcept { return hasFlag(kFlagArgHome); }
  ASMJIT_INLINE bool isRegHome() const noexcept { return hasFlag(kFlagRegHome); }

  ASMJIT_INLINE void setAbs(bool value = false) noexcept {
    _mem.flags = static_cast<uint8_t>((_mem.flags & ~kFlagAbs) | (value ? kFlagAbs : 0));
  }
  ASMJIT_INLINE void markArgHome() noexcept { addFlags(kFlagArgHome); }
  ASMJIT_INLINE void markRegHome() noexcept { addFlags(kFlagRegHome); }

  ASMJIT_INLINE void clearAbs(bool value = false) noexcept { _mem.flags &= ~kFlagAbs; }
  ASMJIT_INLINE void clearArgHome() noexcept { clearFlags(kFlagArgHome); }
  ASMJIT_INLINE void clearRegHome() noexcept { clearFlags(kFlagRegHome); }

  //! Get if the memory operand has a BASE register or label specified.
  ASMJIT_INLINE bool hasBase() const noexcept { return getBaseType() != 0; }
  //! Get if the memory operand has an INDEX register specified.
  ASMJIT_INLINE bool hasIndex() const noexcept { return getIndexType() != 0; }

  //! Get whether the memory operand has BASE and INDEX register.
  ASMJIT_INLINE bool hasBaseOrIndex() const noexcept { return _mem.baseIndexType != 0; }
  //! Get whether the memory operand has BASE and INDEX register.
  ASMJIT_INLINE bool hasBaseAndIndex() const noexcept { return (_mem.baseIndexType & 0xF0U) != 0 && (_mem.baseIndexType & 0x0FU) != 0; }

  //! Get if the BASE operand is a register.
  ASMJIT_INLINE bool hasBaseReg() const noexcept {
    // It's hacky, but if we know that 0 and 1 reg-type is never used by any
    // register we can assume that every reg-type will always have at least
    // one of three remaining bits (in a 4-bit nibble) set. This is a good
    // optimization as we don't have to unpack the BASE type from `baseIndexType`.
    return (_mem.baseIndexType & (0xE << kMemBaseTypeShift)) != 0;
  }
  //! Get if the BASE operand is a register.
  ASMJIT_INLINE bool hasIndexReg() const noexcept {
    return (_mem.baseIndexType & (0xE << kMemIndexTypeShift)) != 0;
  }

  //! Get if the BASE operand is a label.
  ASMJIT_INLINE bool hasBaseLabel() const noexcept { return getBaseType() == Label::kLabelTag; }

  //! Get type of a BASE register (0 if this memory operand doesn't use the BASE register).
  //!
  //! NOTE: If the returned type is one (a value never associated to a register
  //! type) the BASE is not register, but it's a label. One equals to `kLabelTag`.
  //! You should always check `hasBaseLabel()` before using `getBaseId()` result.
  ASMJIT_INLINE uint32_t getBaseType() const noexcept { return (_mem.baseIndexType >> kMemBaseTypeShift) & 0xF; }
  //! Get type of an INDEX register (0 if this memory operand doesn't use the INDEX register).
  ASMJIT_INLINE uint32_t getIndexType() const noexcept { return (_mem.baseIndexType >> kMemIndexTypeShift) & 0xF; }

  //! Get both BASE (3:0 bits) and INDEX (7:4 bits) types combined into a single integer.
  //!
  //! NOTE: This is the way these two are stored. You can use `encodeBaseIndex()`
  //! to encode any two types into the format and bit masking to extract the BASE
  //! and INDEX types from the packed integer.
  ASMJIT_INLINE uint32_t getBaseIndexType() const noexcept { return _mem.baseIndexType; }

  //! Get id of the BASE register or label (if the BASE was specified as label).
  ASMJIT_INLINE uint32_t getBaseId() const noexcept { return _mem.base; }

  //! Get id of the INDEX register.
  ASMJIT_INLINE uint32_t getIndexId() const noexcept { return _mem.index; }

  ASMJIT_INLINE void _setBase(uint32_t regType, uint32_t id) noexcept {
    _mem.baseIndexType = static_cast<uint8_t>(
      (_mem.baseIndexType & ~encodeBaseIndex(0xF, 0)) | encodeBaseIndex(regType, 0));
    _mem.base = id;
  }

  ASMJIT_INLINE void _setIndex(uint32_t regType, uint32_t id) noexcept {
    _mem.baseIndexType = static_cast<uint8_t>(
      (_mem.baseIndexType & ~encodeBaseIndex(0, 0xF)) | encodeBaseIndex(0, regType));
    _mem.index = id;
  }

  ASMJIT_INLINE void setBase(const Reg& base) noexcept { return _setBase(base.getRegType(), base.getId()); }
  ASMJIT_INLINE void setIndex(const Reg& index) noexcept { return _setIndex(index.getRegType(), index.getId()); }

  //! Reset the memory operand's BASE register / label.
  ASMJIT_INLINE void resetBase() noexcept { _setBase(0, 0); }
  //! Reset the memory operand's INDEX register.
  ASMJIT_INLINE void resetIndex() noexcept { _setIndex(0, kInvalidValue); }

  //! Set memory operand size.
  ASMJIT_INLINE void setSize(uint32_t size) noexcept { _mem.size = static_cast<uint8_t>(size); }

  ASMJIT_INLINE bool hasOffset() const noexcept {
    int32_t lo = static_cast<int32_t>(_mem.offsetLo32);
    int32_t hi = static_cast<int32_t>(_mem.base) & -static_cast<int32_t>(getBaseType() == 0);
    return (lo | hi) != 0;
  }

  //! Get if the memory operand has 64-bit offset or absolute address.
  //!
  //! If this is true then `hasBase()` must always report false.
  ASMJIT_INLINE bool has64BitOffset() const noexcept { return getBaseType() == 0; }

  //! Get a 64-bit offset or absolute address.
  ASMJIT_INLINE int64_t getOffset() const noexcept {
    return has64BitOffset()
      ? static_cast<int64_t>(_mem.offset64)
      : static_cast<int64_t>(static_cast<int32_t>(_mem.offsetLo32)); // Sign-Extend.
  }

  //! Get a lower part of a 64-bit offset or absolute address.
  ASMJIT_INLINE int32_t getOffsetLo32() const noexcept { return static_cast<int32_t>(_mem.offsetLo32); }
  //! Get a higher part of a 64-bit offset or absolute address.
  //!
  //! NOTE: This function is UNSAFE and returns garbage if `has64BitOffset()`
  //! returns false. Never use blindly without checking it.
  ASMJIT_INLINE int32_t getOffsetHi32() const noexcept { return static_cast<int32_t>(_mem.base); }

  //! Set a 64-bit offset or an absolute address to `offset`.
  //!
  //! NOTE: This functions attempts to set both high and low parts of a 64-bit
  //! offset, however, if the operand has a BASE register it will store only the
  //! low 32 bits of the offset / address as there is no way to store both BASE
  //! and 64-bit offset, and there is currently no architecture that has such
  //! capability targeted by AsmJit.
  ASMJIT_INLINE void setOffset(int64_t offset) noexcept {
    if (has64BitOffset())
      _mem.offset64 = static_cast<uint64_t>(offset);
    else
      _mem.offsetLo32 = static_cast<int32_t>(offset & 0xFFFFFFFF);
  }
  //! Adjust the offset by a 64-bit `off`.
  ASMJIT_INLINE void addOffset(int64_t off) noexcept {
    if (has64BitOffset())
      _mem.offset64 += static_cast<uint64_t>(off);
    else
      _mem.offsetLo32 += static_cast<uint32_t>(off & 0xFFFFFFFF);
  }
  //! Reset the memory offset to zero.
  ASMJIT_INLINE void resetOffset() noexcept { setOffset(0); }

  //! Set a low 32-bit offset to `off`.
  ASMJIT_INLINE void setOffsetLo32(int32_t off) noexcept {
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }
  //! Adjust the offset by `off`.
  //!
  //! NOTE: This is a fast function that doesn't use the HI 32-bits of a
  //! 64-bit offset. Use it only if you know that there is a BASE register
  //! and the offset is only 32 bits anyway.
  ASMJIT_INLINE void addOffsetLo32(int32_t off) noexcept {
    _mem.offsetLo32 += static_cast<uint32_t>(off);
  }
  //! Reset the memory offset to zero.
  ASMJIT_INLINE void resetOffsetLo32() noexcept { setOffsetLo32(0); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Mem& operator=(const Mem& other) noexcept { copyFrom(other); return *this; }
};

// ============================================================================
// [asmjit::Imm]
// ============================================================================

//! Immediate operand.
//!
//! Immediate operand is usually part of instruction itself. It's inlined after
//! or before the instruction opcode. Immediates can be only signed or unsigned
//! integers.
//!
//! To create immediate operand use `imm()` or `imm_u()` non-members or `Imm`
//! constructors.
class Imm : public Operand {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new immediate value (initial value is 0).
  Imm() noexcept : Operand(NoInit) {
    _init_packed_op_b1_b2_sz_id(kOpImm, 0, 0, 0, kInvalidValue);
    _imm.value.i64 = 0;
  }

  //! Create a new signed immediate value, assigning the value to `val`.
  explicit Imm(int64_t val) noexcept : Operand(NoInit) {
    _init_packed_op_b1_b2_sz_id(kOpImm, 0, 0, 0, kInvalidValue);
    _imm.value.i64 = val;
  }

  //! Create a new immediate value from `other`.
  ASMJIT_INLINE Imm(const Imm& other) noexcept : Operand(other) {}

  explicit ASMJIT_INLINE Imm(const _NoInit&) noexcept : Operand(NoInit) {}

  // --------------------------------------------------------------------------
  // [Immediate Specific]
  // --------------------------------------------------------------------------

  //! Clone `Imm` operand.
  ASMJIT_INLINE Imm clone() const noexcept { return Imm(*this); }

  //! Get whether the immediate can be casted to 8-bit signed integer.
  ASMJIT_INLINE bool isInt8() const noexcept { return Utils::isInt8(_imm.value.i64); }
  //! Get whether the immediate can be casted to 8-bit unsigned integer.
  ASMJIT_INLINE bool isUInt8() const noexcept { return Utils::isUInt8(_imm.value.i64); }

  //! Get whether the immediate can be casted to 16-bit signed integer.
  ASMJIT_INLINE bool isInt16() const noexcept { return Utils::isInt16(_imm.value.i64); }
  //! Get whether the immediate can be casted to 16-bit unsigned integer.
  ASMJIT_INLINE bool isUInt16() const noexcept { return Utils::isUInt16(_imm.value.i64); }

  //! Get whether the immediate can be casted to 32-bit signed integer.
  ASMJIT_INLINE bool isInt32() const noexcept { return Utils::isInt32(_imm.value.i64); }
  //! Get whether the immediate can be casted to 32-bit unsigned integer.
  ASMJIT_INLINE bool isUInt32() const noexcept { return Utils::isUInt32(_imm.value.i64); }

  //! Get immediate value as 8-bit signed integer.
  ASMJIT_INLINE int8_t getInt8() const noexcept { return static_cast<int8_t>(_imm.value.i32Lo & 0xFF); }
  //! Get immediate value as 8-bit unsigned integer.
  ASMJIT_INLINE uint8_t getUInt8() const noexcept { return static_cast<uint8_t>(_imm.value.u32Lo & 0xFFU); }
  //! Get immediate value as 16-bit signed integer.
  ASMJIT_INLINE int16_t getInt16() const noexcept { return static_cast<int16_t>(_imm.value.i32Lo & 0xFFFF);}
  //! Get immediate value as 16-bit unsigned integer.
  ASMJIT_INLINE uint16_t getUInt16() const noexcept { return static_cast<uint16_t>(_imm.value.u32Lo & 0xFFFFU);}

  //! Get immediate value as 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32() const noexcept { return _imm.value.i32Lo; }
  //! Get low 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32Lo() const noexcept { return _imm.value.i32Lo; }
  //! Get high 32-bit signed integer.
  ASMJIT_INLINE int32_t getInt32Hi() const noexcept { return _imm.value.i32Hi; }

  //! Get immediate value as 32-bit unsigned integer.
  ASMJIT_INLINE uint32_t getUInt32() const noexcept { return _imm.value.u32Lo; }
  //! Get low 32-bit signed integer.
  ASMJIT_INLINE uint32_t getUInt32Lo() const noexcept { return _imm.value.u32Lo; }
  //! Get high 32-bit signed integer.
  ASMJIT_INLINE uint32_t getUInt32Hi() const noexcept { return _imm.value.u32Hi; }

  //! Get immediate value as 64-bit signed integer.
  ASMJIT_INLINE int64_t getInt64() const noexcept { return _imm.value.i64; }
  //! Get immediate value as 64-bit unsigned integer.
  ASMJIT_INLINE uint64_t getUInt64() const noexcept { return _imm.value.u64; }

  //! Get immediate value as `intptr_t`.
  ASMJIT_INLINE intptr_t getIntPtr() const noexcept {
    if (sizeof(intptr_t) == sizeof(int64_t))
      return static_cast<intptr_t>(getInt64());
    else
      return static_cast<intptr_t>(getInt32());
  }

  //! Get immediate value as `uintptr_t`.
  ASMJIT_INLINE uintptr_t getUIntPtr() const noexcept {
    if (sizeof(uintptr_t) == sizeof(uint64_t))
      return static_cast<uintptr_t>(getUInt64());
    else
      return static_cast<uintptr_t>(getUInt32());
  }

  //! Set immediate value to 8-bit signed integer `val`.
  ASMJIT_INLINE void setInt8(int8_t val) noexcept { _imm.value.i64 = static_cast<int64_t>(val); }
  //! Set immediate value to 8-bit unsigned integer `val`.
  ASMJIT_INLINE void setUInt8(uint8_t val) noexcept { _imm.value.u64 = static_cast<uint64_t>(val); }

  //! Set immediate value to 16-bit signed integer `val`.
  ASMJIT_INLINE void setInt16(int16_t val) noexcept { _imm.value.i64 = static_cast<int64_t>(val); }
  //! Set immediate value to 16-bit unsigned integer `val`.
  ASMJIT_INLINE void setUInt16(uint16_t val) noexcept { _imm.value.u64 = static_cast<uint64_t>(val); }

  //! Set immediate value to 32-bit signed integer `val`.
  ASMJIT_INLINE void setInt32(int32_t val) noexcept { _imm.value.i64 = static_cast<int64_t>(val); }
  //! Set immediate value to 32-bit unsigned integer `val`.
  ASMJIT_INLINE void setUInt32(uint32_t val) noexcept { _imm.value.u64 = static_cast<uint64_t>(val); }

  //! Set immediate value to 64-bit signed integer `val`.
  ASMJIT_INLINE void setInt64(int64_t val) noexcept { _imm.value.i64 = val; }
  //! Set immediate value to 64-bit unsigned integer `val`.
  ASMJIT_INLINE void setUInt64(uint64_t val) noexcept { _imm.value.u64 = val; }
  //! Set immediate value to intptr_t `val`.
  ASMJIT_INLINE void setIntPtr(intptr_t val) noexcept { _imm.value.i64 = static_cast<int64_t>(val); }
  //! Set immediate value to uintptr_t `val`.
  ASMJIT_INLINE void setUIntPtr(uintptr_t val) noexcept { _imm.value.u64 = static_cast<uint64_t>(val); }

  //! Set immediate value as unsigned type to `val`.
  ASMJIT_INLINE void setPtr(void* p) noexcept { setIntPtr((uint64_t)p); }
  //! Set immediate value to `val`.
  template<typename T>
  ASMJIT_INLINE void setValue(T val) noexcept { setIntPtr((int64_t)val); }

  // --------------------------------------------------------------------------
  // [Float]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void setFloat(float f) noexcept {
    _imm.value.f32Lo = f;
    _imm.value.u32Hi = 0;
  }

  ASMJIT_INLINE void setDouble(double d) noexcept {
    _imm.value.f64 = d;
  }

  // --------------------------------------------------------------------------
  // [Truncate]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void truncateTo8Bits() noexcept {
    if (ASMJIT_ARCH_64BIT) {
      _imm.value.u64   &= static_cast<uint64_t>(0x000000FFU);
    }
    else {
      _imm.value.u32Lo &= 0x000000FFU;
      _imm.value.u32Hi  = 0;
    }
  }

  ASMJIT_INLINE void truncateTo16Bits() noexcept {
    if (ASMJIT_ARCH_64BIT) {
      _imm.value.u64   &= static_cast<uint64_t>(0x0000FFFFU);
    }
    else {
      _imm.value.u32Lo &= 0x0000FFFFU;
      _imm.value.u32Hi  = 0;
    }
  }

  ASMJIT_INLINE void truncateTo32Bits() noexcept { _imm.value.u32Hi = 0; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  //! Assign `other` to the immediate operand.
  ASMJIT_INLINE Imm& operator=(const Imm& other) noexcept { copyFrom(other); return *this; }
};

//! Create a signed immediate operand.
static ASMJIT_INLINE Imm imm(int64_t val) noexcept { return Imm(val); }
//! Create an unsigned immediate operand.
static ASMJIT_INLINE Imm imm_u(uint64_t val) noexcept { return Imm(static_cast<int64_t>(val)); }
//! Create an immediate operand from `p`.
template<typename T>
static ASMJIT_INLINE Imm imm_ptr(T p) noexcept { return Imm(static_cast<int64_t>((intptr_t)p)); }

// ============================================================================
// [asmjit::TypeId]
// ============================================================================

//! Type-id.
//!
//! This is an additional information that can be used to describe a physical
//! or virtual register. it's used mostly by CodeCompiler to describe register
//! representation (the kind of data stored in the register and the width used)
//! and it's also used by APIs that allow to describe and work with function
//! signatures.
struct TypeId {
  // --------------------------------------------------------------------------
  // [Id]
  // --------------------------------------------------------------------------

  enum Id {
    kVoid         = 0,

    _kIntStart    = 32,
    _kIntEnd      = 41,

    kIntPtr       = 32,
    kUIntPtr      = 33,

    kI8           = 34,
    kU8           = 35,
    kI16          = 36,
    kU16          = 37,
    kI32          = 38,
    kU32          = 39,
    kI64          = 40,
    kU64          = 41,

    _kFloatStart  = 42,
    _kFloatEnd    = 44,

    kF32          = 42,
    kF64          = 43,
    kF80          = 44,

    _kMaskStart   = 45,
    _kMaskEnd     = 48,

    kMask8        = 45,
    kMask16       = 46,
    kMask32       = 47,
    kMask64       = 48,

    _kMmxStart    = 49,
    _kMmxEnd      = 50,

    kMmx32        = 49,
    kMmx64        = 50,

    _kVec32Start  = 51,
    _kVec32End    = 60,

    kI8x4         = 51,
    kU8x4         = 52,
    kI16x2        = 53,
    kU16x2        = 54,
    kI32x1        = 55,
    kU32x1        = 56,
    kF32x1        = 59,

    _kVec64Start  = 61,
    _kVec64End    = 70,

    kI8x8         = 61,
    kU8x8         = 62,
    kI16x4        = 63,
    kU16x4        = 64,
    kI32x2        = 65,
    kU32x2        = 66,
    kI64x1        = 67,
    kU64x1        = 68,
    kF32x2        = 69,
    kF64x1        = 70,

    _kVec128Start = 71,
    _kVec128End   = 80,

    kI8x16        = 71,
    kU8x16        = 72,
    kI16x8        = 73,
    kU16x8        = 74,
    kI32x4        = 75,
    kU32x4        = 76,
    kI64x2        = 77,
    kU64x2        = 78,
    kF32x4        = 79,
    kF64x2        = 80,

    _kVec256Start = 81,
    _kVec256End   = 90,

    kI8x32        = 81,
    kU8x32        = 82,
    kI16x16       = 83,
    kU16x16       = 84,
    kI32x8        = 85,
    kU32x8        = 86,
    kI64x4        = 87,
    kU64x4        = 88,
    kF32x8        = 89,
    kF64x4        = 90,

    _kVec512Start = 91,
    _kVec512End   = 100,

    kI8x64        = 91,
    kU8x64        = 92,
    kI16x32       = 93,
    kU16x32       = 94,
    kI32x16       = 95,
    kU32x16       = 96,
    kI64x8        = 97,
    kU64x8        = 98,
    kF32x16       = 99,
    kF64x8        = 100,

    kCount        = 101
  };

  // --------------------------------------------------------------------------
  // [TypeName - Used by Templates]
  // --------------------------------------------------------------------------

  struct Int8    {};                     //!< int8_t as C++ type-name.
  struct UInt8   {};                     //!< uint8_t as C++ type-name.
  struct Int16   {};                     //!< int16_t as C++ type-name.
  struct UInt16  {};                     //!< uint16_t as C++ type-name.
  struct Int32   {};                     //!< int32_t as C++ type-name.
  struct UInt32  {};                     //!< uint32_t as C++ type-name.
  struct Int64   {};                     //!< int64_t as C++ type-name.
  struct UInt64  {};                     //!< uint64_t as C++ type-name.
  struct IntPtr  {};                     //!< intptr_t as C++ type-name.
  struct UIntPtr {};                     //!< uintptr_t as C++ type-name.
  struct Float   {};                     //!< float as C++ type-name.
  struct Double  {};                     //!< double as C++ type-name.
  struct MmxReg  {};                     //!< MMX register as C++ type-name.
  struct Vec128  {};                     //!< SIMD128/XMM register as C++ type-name.
  struct Vec256  {};                     //!< SIMD256/YMM register as C++ type-name.
  struct Vec512  {};                     //!< SIMD512/ZMM register as C++ type-name.

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  struct Info {
    uint8_t sizeOf[128];
    uint8_t elementOf[128];
  };

  ASMJIT_API static const Info _info;

  static ASMJIT_INLINE bool isVoid(uint32_t typeId) noexcept { return typeId == 0; }
  static ASMJIT_INLINE bool isValid(uint32_t typeId) noexcept { return typeId >= _kIntStart && typeId <= _kVec512End; }
  static ASMJIT_INLINE bool isAbstract(uint32_t typeId) noexcept { return typeId >= kIntPtr && typeId <= kUIntPtr; }
  static ASMJIT_INLINE bool isInt(uint32_t typeId) noexcept { return typeId >= _kIntStart && typeId <= _kIntEnd; }
  static ASMJIT_INLINE bool isGpb(uint32_t typeId) noexcept { return typeId >= kI8 && typeId <= kU8; }
  static ASMJIT_INLINE bool isGpw(uint32_t typeId) noexcept { return typeId >= kI16 && typeId <= kU16; }
  static ASMJIT_INLINE bool isGpd(uint32_t typeId) noexcept { return typeId >= kI32 && typeId <= kU32; }
  static ASMJIT_INLINE bool isGpq(uint32_t typeId) noexcept { return typeId >= kI64 && typeId <= kU64; }
  static ASMJIT_INLINE bool isFloat(uint32_t typeId) noexcept { return typeId >= _kFloatStart && typeId <= _kFloatEnd; }
  static ASMJIT_INLINE bool isMask(uint32_t typeId) noexcept { return typeId >= _kMaskStart && typeId <= _kMaskEnd; }
  static ASMJIT_INLINE bool isMmx(uint32_t typeId) noexcept { return typeId >= _kMmxStart && typeId <= _kMmxEnd; }

  static ASMJIT_INLINE bool isVec(uint32_t typeId) noexcept { return typeId >= _kVec32Start && typeId <= _kVec512End; }
  static ASMJIT_INLINE bool isVec32(uint32_t typeId) noexcept { return typeId >= _kVec32Start && typeId <= _kVec32End; }
  static ASMJIT_INLINE bool isVec64(uint32_t typeId) noexcept { return typeId >= _kVec64Start && typeId <= _kVec64End; }
  static ASMJIT_INLINE bool isVec128(uint32_t typeId) noexcept { return typeId >= _kVec128Start && typeId <= _kVec128End; }
  static ASMJIT_INLINE bool isVec256(uint32_t typeId) noexcept { return typeId >= _kVec256Start && typeId <= _kVec256End; }
  static ASMJIT_INLINE bool isVec512(uint32_t typeId) noexcept { return typeId >= _kVec512Start && typeId <= _kVec512End; }

  static ASMJIT_INLINE uint32_t sizeOf(uint32_t typeId) noexcept {
    ASMJIT_ASSERT(typeId < ASMJIT_ARRAY_SIZE(_info.sizeOf));
    return _info.sizeOf[typeId];
  }

  static ASMJIT_INLINE uint32_t elementOf(uint32_t typeId) noexcept {
    ASMJIT_ASSERT(typeId < ASMJIT_ARRAY_SIZE(_info.elementOf));
    return _info.elementOf[typeId];
  }

  //! Get an offset to convert a `kIntPtr` and `kUIntPtr` TypeId into a
  //! type that matches `gpSize` (general-purpose register size). If you
  //! find such TypeId it's then only about adding the offset to it.
  //!
  //! For example:
  //! ~~~
  //! uint32_t gpSize = '4' or '8';
  //! uint32_t deabstractDelta = TypeId::deabstractDeltaOfSize(gpSize);
  //!
  //! uint32_t typeId = 'some type-id';
  //!
  //! // Normalize some typeId into a non-abstract typeId.
  //! if (TypeId::isAbstract(typeId)) typeId += deabstractDelta;
  //!
  //! // The same, but by using TypeId::deabstract() function.
  //! typeId = TypeId::deabstract(typeId, deabstractDelta);
  //!
  //! ~~~
  static ASMJIT_INLINE uint32_t deabstractDeltaOfSize(uint32_t gpSize) noexcept {
    return gpSize >= 8 ? kI64 - kIntPtr : kI32 - kIntPtr;
  }

  static ASMJIT_INLINE uint32_t deabstract(uint32_t typeId, uint32_t deabstractDelta) noexcept {
    return TypeId::isAbstract(typeId) ? typeId += deabstractDelta : typeId;
  }
};

//! TypeIdOf<> template allows to get a TypeId of a C++ type.
template<typename T> struct TypeIdOf {
  // Don't provide anything if not specialized.
};
template<typename T> struct TypeIdOf<T*> {
  enum { kTypeId = TypeId::kUIntPtr };
};

template<typename T>
struct TypeIdOfInt {
  enum {
    kSigned = int(~static_cast<T>(0) < static_cast<T>(0)),
    kTypeId = (sizeof(T) == 1) ? (int)(kSigned ? TypeId::kI8  : TypeId::kU8 ) :
              (sizeof(T) == 2) ? (int)(kSigned ? TypeId::kI16 : TypeId::kU16) :
              (sizeof(T) == 4) ? (int)(kSigned ? TypeId::kI32 : TypeId::kU32) :
              (sizeof(T) == 8) ? (int)(kSigned ? TypeId::kI64 : TypeId::kU64) : (int)TypeId::kVoid
  };
};

#define ASMJIT_DEFINE_TYPE_ID(T, TYPE_INFO) \
  template<> struct TypeIdOf<T> { enum { kTypeId = TYPE_INFO }; }

ASMJIT_DEFINE_TYPE_ID(signed char       , TypeIdOfInt<signed char>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned char     , TypeIdOfInt<unsigned char>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(short             , TypeIdOfInt<short>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned short    , TypeIdOfInt<unsigned short>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(int               , TypeIdOfInt<int>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned int      , TypeIdOfInt<unsigned int>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(long              , TypeIdOfInt<long>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned long     , TypeIdOfInt<unsigned long>::kTypeId);
#if ASMJIT_CC_MSC && !ASMJIT_CC_MSC_GE(16, 0, 0)
ASMJIT_DEFINE_TYPE_ID(__int64           , TypeIdOfInt<__int64>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned __int64  , TypeIdOfInt<unsigned __int64>::kTypeId);
#else
ASMJIT_DEFINE_TYPE_ID(long long         , TypeIdOfInt<long long>::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned long long, TypeIdOfInt<unsigned long long>::kTypeId);
#endif
#if ASMJIT_CC_HAS_NATIVE_CHAR
ASMJIT_DEFINE_TYPE_ID(char              , TypeIdOfInt<char>::kTypeId);
#endif
#if ASMJIT_CC_HAS_NATIVE_CHAR16_T
ASMJIT_DEFINE_TYPE_ID(char16_t          , TypeIdOfInt<char16_t>::kTypeId);
#endif
#if ASMJIT_CC_HAS_NATIVE_CHAR32_T
ASMJIT_DEFINE_TYPE_ID(char32_t          , TypeIdOfInt<char32_t>::kTypeId);
#endif
#if ASMJIT_CC_HAS_NATIVE_WCHAR_T
ASMJIT_DEFINE_TYPE_ID(wchar_t           , TypeIdOfInt<wchar_t>::kTypeId);
#endif

ASMJIT_DEFINE_TYPE_ID(void              , TypeId::kVoid);
ASMJIT_DEFINE_TYPE_ID(float             , TypeId::kF32);
ASMJIT_DEFINE_TYPE_ID(double            , TypeId::kF64);

ASMJIT_DEFINE_TYPE_ID(TypeId::Int8      , TypeId::kI8);
ASMJIT_DEFINE_TYPE_ID(TypeId::UInt8     , TypeId::kU8);
ASMJIT_DEFINE_TYPE_ID(TypeId::Int16     , TypeId::kI16);
ASMJIT_DEFINE_TYPE_ID(TypeId::UInt16    , TypeId::kU16);
ASMJIT_DEFINE_TYPE_ID(TypeId::Int32     , TypeId::kI32);
ASMJIT_DEFINE_TYPE_ID(TypeId::UInt32    , TypeId::kU32);
ASMJIT_DEFINE_TYPE_ID(TypeId::Int64     , TypeId::kI64);
ASMJIT_DEFINE_TYPE_ID(TypeId::UInt64    , TypeId::kU64);
ASMJIT_DEFINE_TYPE_ID(TypeId::IntPtr    , TypeId::kIntPtr);
ASMJIT_DEFINE_TYPE_ID(TypeId::UIntPtr   , TypeId::kUIntPtr);
ASMJIT_DEFINE_TYPE_ID(TypeId::Float     , TypeId::kF32);
ASMJIT_DEFINE_TYPE_ID(TypeId::Double    , TypeId::kF64);
ASMJIT_DEFINE_TYPE_ID(TypeId::MmxReg    , TypeId::kMmx64);
ASMJIT_DEFINE_TYPE_ID(TypeId::Vec128    , TypeId::kI32x4);
ASMJIT_DEFINE_TYPE_ID(TypeId::Vec256    , TypeId::kI32x8);
ASMJIT_DEFINE_TYPE_ID(TypeId::Vec512    , TypeId::kI32x16);

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_OPERAND_H
