// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_INST_H
#define _ASMJIT_CORE_INST_H

// [Dependencies]
#include "../core/cpuinfo.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
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
    kRead                 = 0x00000001U, //!< Operand is Read.
    kWrite                = 0x00000002U, //!< Operand is Written.
    kRW                   = 0x00000003U, //!< Operand is ReadWrite.
    kUse                  = 0x00000004U, //!< Operand is either Read or ReadWrite.
    kOut                  = 0x00000008U  //!< Operand is always WriteOnly (not Read nor ReadWrite).
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline uint32_t getFlags() const noexcept { return _flags; }

  inline bool isRead() const noexcept { return hasFlag(kRead); }
  inline bool isWrite() const noexcept { return hasFlag(kWrite); }

  inline bool isReadOnly() const noexcept { return (_flags & kRW) == kRead; }
  inline bool isWriteOnly() const noexcept { return (_flags & kRW) == kWrite; }
  inline bool isReadWrite() const noexcept { return (_flags & kRW) == kRW; }

  inline bool isUse() const noexcept { return hasFlag(kUse); }
  inline bool isOut() const noexcept { return hasFlag(kOut); }

  inline bool hasPhysId() const noexcept { return _physId != Reg::kIdBad; }
  inline uint32_t getPhysId() const noexcept { return _physId; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint16_t _flags;                       //!< Operand flags, see \ref Flags.
  uint8_t _physId;                       //!< Operand must be in this physical register.
  uint8_t _reserved;                     //!< Reserved for future use.
};

// ============================================================================
// [asmjit::Inst]
// ============================================================================

//! Definitions and utilities related to instructions used by all architectures.
struct Inst {
  // --------------------------------------------------------------------------
  // [Id]
  // --------------------------------------------------------------------------

  enum Id : uint32_t {
    kIdNone               = 0x00000000U, //!< Invalid or uninitialized instruction id.
    kIdAbstract           = 0x80000000U  //!< Abstract instruction (CodeBuilder and CodeCompiler).
  };

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  enum Options : uint32_t {
    //! Reserved for use by CodeBuilder and CodeCompiler.
    kOptionReserved       = 0x00000001U,

    //! Used only by Assembler to mark that `_op4` and `_op5` are used (internal).
    kOptionOp4Op5Used     = 0x00000002U,

    //! Prevents following a jump during compilation (CodeCompiler).
    kOptionUnfollow       = 0x00000010U,

    //! Overwrite the destination operand(s) (CodeCompiler).
    //!
    //! Hint that is important for register liveness analysis. It tells the
    //! compiler that the destination operand will be overwritten now or by
    //! adjacent instructions. CodeCompiler knows when a register is completely
    //! overwritten by a single instruction, for example you don't have to
    //! mark "movaps" or "pxor x, x", however, if a pair of instructions is
    //! used and the first of them doesn't completely overwrite the content
    //! of the destination, CodeCompiler fails to mark that register as dead.
    //!
    //! X86/X64 Specific
    //! ----------------
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
    //!     there is no old content require `CodeCompiler::overwrite()` to be used.
    //!     Some examples (not always the best use cases thought):
    //!
    //!     - `movlps xmm0, ?` followed by `movhps xmm0, ?` and vice versa
    //!     - `movlpd xmm0, ?` followed by `movhpd xmm0, ?` and vice versa
    //!     - `mov al, ?` followed by `and ax, 0xFF`
    //!     - `mov al, ?` followed by `mov ah, al`
    //!     - `pinsrq xmm0, ?, 0` followed by `pinsrq xmm0, ?, 1`
    //!
    //!   - If allocated variable is used temporarily for scalar operations. For
    //!     example if you allocate a full vector like `X86Compiler::newXmm()`
    //!     and then use that vector for scalar operations you should use
    //!     `overwrite()` directive:
    //!
    //!     - `sqrtss x, y` - only LO element of `x` is changed, if you don't use
    //!       HI elements, use `X86Compiler.overwrite().sqrtss(x, y)`.
    kOptionOverwrite      = 0x00000020U,

    kOptionShortForm      = 0x00000040U, //!< Emit short-form of the instruction.
    kOptionLongForm       = 0x00000080U, //!< Emit long-form of the instruction.

    kOptionTaken          = 0x00000100U, //!< Conditional jump is likely to be taken.
    kOptionNotTaken       = 0x00000200U  //!< Conditional jump is unlikely to be taken.
  };

  // --------------------------------------------------------------------------
  // [ControlType]
  // --------------------------------------------------------------------------

  //! Control-flow type.
  enum ControlType : uint32_t {
    kControlRegular       = 0U,          //!< Regular control flow.
    kControlJump          = 1U,          //!< Unconditional jump.
    kControlBranch        = 2U,          //!< Conditional jump (branch).
    kControlCall          = 3U,          //!< Function call.
    kControlReturn        = 4U           //!< Function return.
  };

  // --------------------------------------------------------------------------
  // [ORWInfo / IRWInfo]
  // --------------------------------------------------------------------------

  //! Operand Read/Write information, used by \ref IRWInfo.
  struct ORWInfo {
    //! Flags describe how the operand is accessed and some additional information.
    enum Flags : uint32_t {
      kRead               = 0x00000001U, //!< Operand is Read.
      kWrite              = 0x00000002U, //!< Operand is Written.
      kRW                 = 0x00000003U, //!< Operand is ReadWrite.
      kUse                = 0x00000004U, //!< Operand is either Read or ReadWrite.
      kOut                = 0x00000008U, //!< Operand is always WriteOnly (not Read nor ReadWrite).
      kZExt               = 0x00000010U  //!< The output is zero extended to a native register size.
    };

    inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
    inline uint32_t getFlags() const noexcept { return _flags; }

    inline bool isRead() const noexcept { return hasFlag(kRead); }
    inline bool isWrite() const noexcept { return hasFlag(kWrite); }
    inline bool isReadWrite() const noexcept { return (_flags & kRW) == kRW; }
    inline bool isReadOnly() const noexcept { return (_flags & kRW) == kRead; }
    inline bool isWriteOnly() const noexcept { return (_flags & kRW) == kWrite; }

    inline bool isUse() const noexcept { return hasFlag(kUse); }
    inline bool isOut() const noexcept { return hasFlag(kOut); }

    inline bool hasPhysId() const noexcept { return _physId != Reg::kIdBad; }
    inline uint32_t getPhysId() const noexcept { return _physId; }

    inline uint32_t getIndex() const noexcept { return _index; }
    inline uint32_t getWidth() const noexcept { return _width; }

    uint8_t _flags;                      //!< Read/Write flags.
    uint8_t _physId;                     //!< Physical register index, if required.
    uint8_t _index;                      //!< Read/write register index [in bytes], `_index` is ignored if the operand is memory.
    uint8_t _width;                      //!< Read/Write register/memory width [in bytes], zero means native width or imm/rel width.
  };

  //! Instruction Read/Write information.
  struct IRWInfo {
    uint32_t flags;
    ORWInfo extraReg;
    ORWInfo operands[Globals::kMaxOpCount];
  };

  // --------------------------------------------------------------------------
  // [Detail]
  // --------------------------------------------------------------------------

  //! Instruction id, options, and extraReg packed in a single structure. This
  //! structure exists to simplify analysis and validation API that requires a
  //! lot of information about the instruction to be processed.
  class Detail {
  public:
    inline Detail() noexcept
      : instId(0),
        options(0),
        extraReg() {}

    explicit inline Detail(uint32_t instId, uint32_t options = 0) noexcept
      : instId(instId),
        options(options),
        extraReg() {}

    inline Detail(uint32_t instId, uint32_t options, const RegOnly& reg) noexcept
      : instId(instId),
        options(options),
        extraReg(reg) {}

    inline Detail(uint32_t instId, uint32_t options, const Reg& reg) noexcept
      : instId(instId),
        options(options) { extraReg.init(reg); }

    // ------------------------------------------------------------------------
    // [Accessors]
    // ------------------------------------------------------------------------

    inline bool hasExtraReg() const noexcept { return extraReg.isValid(); }
    inline void resetExtraReg() noexcept { extraReg.reset(); }

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    uint32_t instId;
    uint32_t options;
    RegOnly extraReg;
  };

  // --------------------------------------------------------------------------
  // [API]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_INST_API
  //! Validate the given instruction.
  ASMJIT_API static Error validate(uint32_t archType, const Detail& detail, const Operand_* operands, uint32_t count) noexcept;

  //! Get Read/Write information of the given instruction.
  ASMJIT_API static Error queryRWInfo(uint32_t archType, const Detail& detail, const Operand_* operands, uint32_t count, IRWInfo& out) noexcept;

  //! Get CPU features required by the given instruction.
  ASMJIT_API static Error queryCpuFeatures(uint32_t archType, const Detail& detail, const Operand_* operands, uint32_t count, CpuFeatures& out) noexcept;
  #endif
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_INST_H
