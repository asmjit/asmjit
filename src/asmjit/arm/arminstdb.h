// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ARM_ARMINSTDB_H
#define _ASMJIT_ARM_ARMINSTDB_H

#include "../core/assembler.h"
#include "../core/operand.h"
#include "../arm/armglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \cond INTERNAL
//! \addtogroup asmjit_arm_db
//! \{

// ============================================================================
// [asmjit::arm::InstDB]
// ============================================================================

//! ARM instruction data (ARM32 and/or THUMBx).
namespace InstDB {

  //! Instruction encodings, used by `arm::Assembler`.
  enum EncodingType : uint32_t {
    kEncodingNone = 0,                   //!< Never used.
    kEncodingCount                       //!< Count of instruction encodings.
  };

  //! Instruction family.
  //!
  //! Specifies which table should be used to interpret `_familyDataIndex`.
  enum FamilyType : uint32_t {
    kFamilyNone           = 0,           //!< General purpose or special instruction.
    kFamilyNeon           = 1            //!< NEON family instruction.
  };

  //! Instruction flags.
  enum InstFlags : uint32_t {
    kInstFlagNone         = 0x00000000u  //!< No flags.
  };

  //! Supported architectures.
  enum Mode : uint32_t {
    kModeArm32            = 0x01,        //!< ARM32 mode supported.
    kModeArm64            = 0x02         //!< ARM64 mode supported.
  };

  //! Common data - aggregated data that is shared across many instructions.
  struct CommonData {
    // ------------------------------------------------------------------------
    // [Accessors]
    // ------------------------------------------------------------------------

    //! Gets all instruction flags, see `InstFlags`.
    inline uint32_t flags() const noexcept { return _flags; }
    //! Gets whether the instruction has a `flag`, see `InstFlags`.
    inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    //! Instruction flags.
    uint32_t _flags;
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the instruction name (null terminated).
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! return an empty string (null terminated string of zero size).
  inline const char* name() const noexcept;

  //! Gets `CommonData` of the instruction.
  inline const CommonData& commonInfo() const noexcept;
  //! Gets index to `ArmInstDB::commonData` of this instruction.
  inline uint32_t commonInfoIndex() const noexcept { return _commonDataIndex; }

  //! Gets the instruction encoding, see `EncodingType`.
  inline uint32_t encodingType() const noexcept { return _encodingType; }

  //! Gets the instruction opcode, see `OpCodeBits`.
  inline uint32_t opcode() const noexcept { return _opCode; }

  //! Gets whether the instruction has flag `flag`, see `InstFlags`.
  inline bool hasFlag(uint32_t flag) const noexcept { return commonInfo().hasFlag(flag); }
  //! Gets the instruction flags, see `InstFlags`.
  inline uint32_t flags() const noexcept { return commonInfo().flags(); }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  //! Gets whether the `instId` is defined (counts also kInvalidInstId, which is zero).
  static inline bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Gets instruction information based on the instruction `instId`.
  //!
  //! NOTE: `instId` has to be a valid instruction ID, it can't be greater than
  //! or equal to `ArmInst::_kIdCount`. It asserts in debug mode.
  static inline const ArmInst& getInst(uint32_t instId) noexcept;

  // --------------------------------------------------------------------------
  // [Id <-> Name]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_TEXT
  //! Gets an instruction ID from a given instruction `name`.
  //!
  //! NOTE: Instruction name MUST BE in lowercase, otherwise there will be no
  //! match. If there is an exact match the instruction id is returned, otherwise
  //! `kInvalidInstId` (zero) is returned instead. The given `name` doesn't have
  //! to be null-terminated if `size` is provided.
  ASMJIT_API static uint32_t idByName(const char* name, size_t size = SIZE_MAX) noexcept;

  //! Gets an instruction name from a given instruction id `instId`.
  ASMJIT_API static const char* nameById(uint32_t instId) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Validation]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_INST_API
  ASMJIT_API static Error validate(
    uint32_t archId, uint32_t instId, uint32_t options,
    const Operand_& opExtra,
    const Operand_* operands, uint32_t count) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _encodingType    : 8;         //!< Instruction encoding.
  uint32_t _nameDataIndex   : 14;        //!< Index to `InstDB::nameData` table.
  uint32_t _commonDataIndex : 10;        //!< Index to `InstDB::commonData` table.
  uint32_t _opCode;                      //!< Instruction opcode.

  ASMJIT_VARAPI const ArmInst instData[];
  ASMJIT_VARAPI const ArmInst::CommonData commonData[];
  ASMJIT_VARAPI const char nameData[];

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_ARM_ARMINSTDB_H
