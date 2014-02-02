// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CPU_H
#define _ASMJIT_BASE_CPU_H

// [Api-Begin]
#include "../base/apibegin.h"
#include "../base/assert.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::kCpuVendor]
// ============================================================================

//! @brief Cpu vendor IDs.
//!
//! Cpu vendor IDs are specific for AsmJit library. Vendor ID is not directly
//! read from cpuid result, instead it's based on CPU vendor string.
ASMJIT_ENUM(kCpuVendor) {
  //! @brief Unknown CPU vendor.
  kCpuVendorUnknown = 0,

  //! @brief Intel CPU vendor.
  kCpuVendorIntel = 1,
  //! @brief AMD CPU vendor.
  kCpuVendorAmd = 2,
  //! @brief National Semiconductor CPU vendor (applies also to Cyrix processors).
  kCpuVendorNSM = 3,
  //! @brief Transmeta CPU vendor.
  kCpuVendorTransmeta = 4,
  //! @brief VIA CPU vendor.
  kCpuVendorVia = 5
};

// ============================================================================
// [asmjit::BaseCpu]
// ============================================================================

//! @brief Base cpu information.
struct BaseCpu {
  ASMJIT_NO_COPY(BaseCpu)

  enum { kFeaturesPerUInt32 = static_cast<int>(sizeof(uint32_t)) * 8 };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseCpu(uint32_t size = sizeof(BaseCpu)) : _size(size) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get CPU vendor string.
  ASMJIT_INLINE const char* getVendorString() const { return _vendorString; }
  //! @brief Get CPU brand string.
  ASMJIT_INLINE const char* getBrandString() const { return _brandString; }

  //! @brief Get CPU vendor ID.
  ASMJIT_INLINE uint32_t getVendorId() const { return _vendorId; }
  //! @brief Get CPU family ID.
  ASMJIT_INLINE uint32_t getFamily() const { return _family; }
  //! @brief Get CPU model ID.
  ASMJIT_INLINE uint32_t getModel() const { return _model; }
  //! @brief Get CPU stepping.
  ASMJIT_INLINE uint32_t getStepping() const { return _stepping; }
  //! @brief Get CPU cores count (or sum of all cores of all procesors).
  ASMJIT_INLINE uint32_t getCoresCount() const { return _coresCount; }

  //! @brief Get whether CPU has a @a feature.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    return static_cast<bool>(
      (_features[feature / kFeaturesPerUInt32] >> (feature % kFeaturesPerUInt32)) & 0x1);
  }

  //! @brief Add CPU @a feature.
  ASMJIT_INLINE BaseCpu& addFeature(uint32_t feature) {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    _features[feature / kFeaturesPerUInt32] |= (1U << (feature % kFeaturesPerUInt32));
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! @brief Detect number of cores (or sum of all cores of all processors).
  static ASMJIT_API uint32_t detectNumberOfCores();

  //! @brief Get host cpu.
  static ASMJIT_API const BaseCpu* getHost();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Size of the structure in bytes.
  uint32_t _size;

  //! @brief Cpu short vendor string.
  char _vendorString[16];
  //! @brief Cpu long vendor string (brand).
  char _brandString[64];

  //! @brief Cpu vendor id (see @c asmjit::kCpuVendor enum).
  uint32_t _vendorId;
  //! @brief Cpu family ID.
  uint32_t _family;
  //! @brief Cpu model ID.
  uint32_t _model;
  //! @brief Cpu stepping.
  uint32_t _stepping;
  //! @brief Cpu cores count (or sum of all CPU cores of all processors).
  uint32_t _coresCount;

  //! @brief Cpu features bitfield.
  uint32_t _features[4];
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CPU_H
