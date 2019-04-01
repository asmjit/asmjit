// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_INST_H
#define _ASMJIT_CORE_INST_H

#include "../core/cpuinfo.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::OpInfo]
// ============================================================================

//! Operand information.
struct OpInfo {
  //! Operand flags.
  //!
  //! Flags describe how the operand is accessed and some additional information.
  enum Flags : uint32_t {
    kRead                 = 0x00000001u, //!< Operand is Read.
    kWrite                = 0x00000002u, //!< Operand is Written.
    kRW                   = 0x00000003u, //!< Operand is ReadWrite.
    kUse                  = 0x00000004u, //!< Operand is either Read or ReadWrite.
    kOut                  = 0x00000008u  //!< Operand is always WriteOnly (not Read nor ReadWrite).
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline uint32_t flags() const noexcept { return _flags; }

  inline bool isRead() const noexcept { return hasFlag(kRead); }
  inline bool isWrite() const noexcept { return hasFlag(kWrite); }

  inline bool isReadOnly() const noexcept { return (_flags & kRW) == kRead; }
  inline bool isWriteOnly() const noexcept { return (_flags & kRW) == kWrite; }
  inline bool isReadWrite() const noexcept { return (_flags & kRW) == kRW; }

  inline bool isUse() const noexcept { return hasFlag(kUse); }
  inline bool isOut() const noexcept { return hasFlag(kOut); }

  inline bool hasPhysId() const noexcept { return _physId != BaseReg::kIdBad; }
  inline uint32_t physId() const noexcept { return _physId; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Operand flags, see `Flags`.
  uint16_t _flags;
  //! Operand must be in this physical register.
  uint8_t _physId;
  //! Reserved for future use.
  uint8_t _reserved;
};

// ============================================================================
// [asmjit::IRWInfo / ORWInfo]
// ============================================================================

//! Read/Write information related to a single operand, used by `IRWInfo`.
struct ORWInfo {
  //! Flags describe how the operand is accessed and some additional information.
  enum Flags : uint32_t {
    kRead                 = 0x00000001u, //!< Operand is read.
    kWrite                = 0x00000002u, //!< Operand is written.
    kRW                   = 0x00000003u, //!< Operand is both read and written.
    kUse                  = 0x00000004u, //!< Operand has a USE slot (each read or read/write operation).
    kOut                  = 0x00000008u, //!< Operand has an OUT slot (each write-only operation).
    kZExt                 = 0x00000010u  //!< The output is zero extended to a native register size.
  };

  inline uint32_t flags() const noexcept { return _flags; }
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  inline bool isRead() const noexcept { return hasFlag(kRead); }
  inline bool isWrite() const noexcept { return hasFlag(kWrite); }
  inline bool isReadWrite() const noexcept { return (_flags & kRW) == kRW; }
  inline bool isReadOnly() const noexcept { return (_flags & kRW) == kRead; }
  inline bool isWriteOnly() const noexcept { return (_flags & kRW) == kWrite; }

  inline bool isUse() const noexcept { return hasFlag(kUse); }
  inline bool isOut() const noexcept { return hasFlag(kOut); }

  inline bool hasPhysId() const noexcept { return _physId != BaseReg::kIdBad; }
  inline uint32_t physId() const noexcept { return _physId; }

  inline uint32_t index() const noexcept { return _index; }
  inline uint32_t width() const noexcept { return _width; }

  //! Read/Write flags.
  uint8_t _flags;
  //! Physical register index, if required.
  uint8_t _physId;
  //! Read/write register index [in bytes], `_index` is ignored if the operand is memory.
  uint8_t _index;
  //! Read/Write register/memory width [in bytes], zero means native width or imm/rel width.
  uint8_t _width;
};

//! Read/Write information related to the whole instruction.
struct IRWInfo {
  uint32_t flags;
  uint32_t opCount;
  ORWInfo extraReg;
  ORWInfo operands[Globals::kMaxOpCount];
};

// ============================================================================
// [asmjit::BaseInst]
// ============================================================================

//! Instruction id, options, and extraReg in a single structure. This structure
//! exists mainly to simplify analysis and validation API that requires `BaseInst`
//! and `Operand[]` array.
class BaseInst {
public:
  enum Id : uint32_t {
    kIdNone               = 0x00000000u, //!< Invalid or uninitialized instruction id.
    kIdAbstract           = 0x80000000u  //!< Abstract instruction (BaseBuilder and BaseCompiler).
  };

  enum Options : uint32_t {
    //! Used internally by emitters for handling errors and rare cases.
    kOptionReserved       = 0x00000001u,

    //! Used only by Assembler to mark that `_op4` and `_op5` are used (internal).
    kOptionOp4Op5Used     = 0x00000002u,

    //! Prevents following a jump during compilation (BaseCompiler).
    kOptionUnfollow       = 0x00000010u,

    //! Overwrite the destination operand(s) (BaseCompiler).
    //!
    //! Hint that is important for register liveness analysis. It tells the
    //! compiler that the destination operand will be overwritten now or by
    //! adjacent instructions. BaseCompiler knows when a register is completely
    //! overwritten by a single instruction, for example you don't have to
    //! mark "movaps" or "pxor x, x", however, if a pair of instructions is
    //! used and the first of them doesn't completely overwrite the content
    //! of the destination, BaseCompiler fails to mark that register as dead.
    //!
    //! X86 Specific
    //! ------------
    //!
    //!   - All instructions that always overwrite at least the size of the
    //!     register the virtual-register uses , for example "mov", "movq",
    //!     "movaps" don't need the overwrite option to be used - conversion,
    //!     shuffle, and other miscellaneous instructions included.
    //!
    //!   - All instructions that clear the destination register if all operands
    //!     are the same, for example "xor x, x", "pcmpeqb x x", etc...
    //!
    //!   - Consecutive instructions that partially overwrite the variable until
    //!     there is no old content require `BaseCompiler::overwrite()` to be used.
    //!     Some examples (not always the best use cases thought):
    //!
    //!     - `movlps xmm0, ?` followed by `movhps xmm0, ?` and vice versa
    //!     - `movlpd xmm0, ?` followed by `movhpd xmm0, ?` and vice versa
    //!     - `mov al, ?` followed by `and ax, 0xFF`
    //!     - `mov al, ?` followed by `mov ah, al`
    //!     - `pinsrq xmm0, ?, 0` followed by `pinsrq xmm0, ?, 1`
    //!
    //!   - If allocated variable is used temporarily for scalar operations. For
    //!     example if you allocate a full vector like `x86::Compiler::newXmm()`
    //!     and then use that vector for scalar operations you should use
    //!     `overwrite()` directive:
    //!
    //!     - `sqrtss x, y` - only LO element of `x` is changed, if you don't
    //!       use HI elements, use `compiler.overwrite().sqrtss(x, y)`.
    kOptionOverwrite      = 0x00000020u,

    kOptionShortForm      = 0x00000040u, //!< Emit short-form of the instruction.
    kOptionLongForm       = 0x00000080u, //!< Emit long-form of the instruction.

    kOptionTaken          = 0x00000100u, //!< Conditional jump is likely to be taken.
    kOptionNotTaken       = 0x00000200u  //!< Conditional jump is unlikely to be taken.
  };

  //! Control type.
  enum ControlType : uint32_t {
    kControlNone          = 0u,          //!< No control type (doesn't jump).
    kControlJump          = 1u,          //!< Unconditional jump.
    kControlBranch        = 2u,          //!< Conditional jump (branch).
    kControlCall          = 3u,          //!< Function call.
    kControlReturn        = 4u           //!< Function return.
  };

  // ------------------------------------------------------------------------
  // [Init / Destroy]
  // ------------------------------------------------------------------------

  inline explicit BaseInst(uint32_t id = 0, uint32_t options = 0) noexcept
    : _id(id),
      _options(options),
      _extraReg() {}

  inline BaseInst(uint32_t id, uint32_t options, const RegOnly& extraReg) noexcept
    : _id(id),
      _options(options),
      _extraReg(extraReg) {}

  inline BaseInst(uint32_t id, uint32_t options, const BaseReg& extraReg) noexcept
    : _id(id),
      _options(options),
      _extraReg { extraReg.signature(), extraReg.id() } {}

  // ------------------------------------------------------------------------
  // [Accessors]
  // ------------------------------------------------------------------------

  inline uint32_t id() const noexcept { return _id; }
  inline void setId(uint32_t id) noexcept { _id = id; }
  inline void resetId() noexcept { _id = 0; }

  inline uint32_t options() const noexcept { return _options; }
  inline void setOptions(uint32_t options) noexcept { _options = options; }
  inline void addOptions(uint32_t options) noexcept { _options |= options; }
  inline void clearOptions(uint32_t options) noexcept { _options &= ~options; }
  inline void resetOptions() noexcept { _options = 0; }

  inline bool hasExtraReg() const noexcept { return _extraReg.isReg(); }
  inline RegOnly& extraReg() noexcept { return _extraReg; }
  inline const RegOnly& extraReg() const noexcept { return _extraReg; }
  inline void setExtraReg(const BaseReg& reg) noexcept { _extraReg.init(reg); }
  inline void setExtraReg(const RegOnly& reg) noexcept { _extraReg.init(reg); }
  inline void resetExtraReg() noexcept { _extraReg.reset(); }

  // --------------------------------------------------------------------------
  // [API]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_INST_API
  //! Validates the given instruction.
  ASMJIT_API static Error validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count) noexcept;

  //! Gets Read/Write information of the given instruction.
  ASMJIT_API static Error queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, IRWInfo& out) noexcept;

  //! Gets CPU features required by the given instruction.
  ASMJIT_API static Error queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, BaseFeatures& out) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _id;
  uint32_t _options;
  RegOnly _extraReg;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_INST_H
