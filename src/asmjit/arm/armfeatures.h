// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ARM_ARMFEATURES_H
#define _ASMJIT_ARM_ARMFEATURES_H

#include "../core/features.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [asmjit::arm::Features]
// ============================================================================

//! CPU features (ARM).
class Features : public BaseFeatures {
public:
  //! CPU feature IDs (ARM).
  enum Id : uint32_t {
    kNone = 0,                 //!< No feature (never set, used internally).
    kARMv6,                    //!< ARMv6 ISA available.
    kARMv7,                    //!< ARMv7 ISA available.
    kARMv8,                    //!< ARMv8 ISA available.
    kTHUMB,                    //!< THUMB v1 ISA available (THUMB mode only).
    kTHUMBv2,                  //!< THUMB v2 ISA available (THUMB mode only).
    kVFPv2,                    //!< CPU has VFPv2 instruction set.
    kVFPv3,                    //!< CPU has VFPv3 instruction set.
    kVFPv4,                    //!< CPU has VFPv4 instruction set.
    kVFP_D32,                  //!< CPU has 32 VFP-D (64-bit) registers.
    kEDSP,                     //!< CPU has EDSP extensions.
    kASIMD,                    //!< CPU has 'Advanced SIMD'.
    kIDIVA,                    //!< CPU has hardware SDIV and UDIV (ARM mode).
    kIDIVT,                    //!< CPU has hardware SDIV and UDIV (THUMB mode).
    kAES,                      //!< CPU has AES instructions (ARM64 only).
    kCRC32,                    //!< CPU has CRC32 instructions.
    kPMULL,                    //!< CPU has PMULL instructions (ARM64 only).
    kSHA1,                     //!< CPU has SHA1 instructions.
    kSHA256,                   //!< CPU has SHA256 instructions.
    kATOMIC64,                 //!< CPU has 64-bit load/store atomics (ARM64 only).
    kCount                     //!< Count of ARM CPU features.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline Features() noexcept
    : BaseFeatures() {}
  inline Features(const Features& other) noexcept
    : BaseFeatures(other) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  #define ASMJIT_ARM_FEATURE(FEATURE) \
    inline bool has##FEATURE() const noexcept { return has(k##FEATURE); }

  ASMJIT_ARM_FEATURE(ARMv6)
  ASMJIT_ARM_FEATURE(ARMv7)
  ASMJIT_ARM_FEATURE(ARMv8)
  ASMJIT_ARM_FEATURE(THUMB)
  ASMJIT_ARM_FEATURE(THUMBv2)
  ASMJIT_ARM_FEATURE(VFPv2)
  ASMJIT_ARM_FEATURE(VFPv3)
  ASMJIT_ARM_FEATURE(VFPv4)
  ASMJIT_ARM_FEATURE(VFP_D32)
  ASMJIT_ARM_FEATURE(EDSP)
  ASMJIT_ARM_FEATURE(ASIMD)
  ASMJIT_ARM_FEATURE(IDIVA)
  ASMJIT_ARM_FEATURE(IDIVT)
  ASMJIT_ARM_FEATURE(AES)
  ASMJIT_ARM_FEATURE(CRC32)
  ASMJIT_ARM_FEATURE(PMULL)
  ASMJIT_ARM_FEATURE(SHA1)
  ASMJIT_ARM_FEATURE(SHA256)
  ASMJIT_ARM_FEATURE(ATOMIC64)

  #undef ASMJIT_ARM_FEATURE

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  inline Features& add(uint32_t featureId) noexcept { return add(featureId).as<Features>(); }
  inline Features& remove(uint32_t featureId) noexcept { return remove(featureId).as<Features>(); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline Features& operator=(const Features& other) noexcept = default;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_ARM_ARMFEATURES_H
