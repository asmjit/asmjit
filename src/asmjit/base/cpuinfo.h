// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CPUINFO_H
#define _ASMJIT_BASE_CPUINFO_H

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base_cpu_info
//! @{

// ============================================================================
// [asmjit::kCpuVendor]
// ============================================================================

//! Cpu vendor IDs.
//!
//! Cpu vendor IDs are specific for AsmJit library. Vendor ID is not directly
//! read from cpuid result, instead it's based on CPU vendor string.
ASMJIT_ENUM(kCpuVendor) {
  //! Unknown CPU vendor.
  kCpuVendorUnknown = 0,

  //! Intel CPU vendor.
  kCpuVendorIntel = 1,
  //! AMD CPU vendor.
  kCpuVendorAmd = 2,
  //! National Semiconductor CPU vendor (applies also to Cyrix processors).
  kCpuVendorNSM = 3,
  //! Transmeta CPU vendor.
  kCpuVendorTransmeta = 4,
  //! VIA CPU vendor.
  kCpuVendorVia = 5
};

// ============================================================================
// [asmjit::BaseCpuInfo]
// ============================================================================

//! Base cpu information.
struct BaseCpuInfo {
  ASMJIT_NO_COPY(BaseCpuInfo)

  //! @internal
  enum {
    kFeaturesPerUInt32 = static_cast<int>(sizeof(uint32_t)) * 8
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BaseCpuInfo(uint32_t size = sizeof(BaseCpuInfo)) : _size(size) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get CPU vendor string.
  ASMJIT_INLINE const char* getVendorString() const { return _vendorString; }
  //! Get CPU brand string.
  ASMJIT_INLINE const char* getBrandString() const { return _brandString; }

  //! Get CPU vendor ID.
  ASMJIT_INLINE uint32_t getVendorId() const { return _vendorId; }
  //! Get CPU family ID.
  ASMJIT_INLINE uint32_t getFamily() const { return _family; }
  //! Get CPU model ID.
  ASMJIT_INLINE uint32_t getModel() const { return _model; }
  //! Get CPU stepping.
  ASMJIT_INLINE uint32_t getStepping() const { return _stepping; }
  //! Get CPU cores count (or sum of all cores of all procesors).
  ASMJIT_INLINE uint32_t getCoresCount() const { return _coresCount; }

  //! Get whether CPU has a `feature`.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    return static_cast<bool>(
      (_features[feature / kFeaturesPerUInt32] >> (feature % kFeaturesPerUInt32)) & 0x1);
  }

  //! Add a CPU `feature`.
  ASMJIT_INLINE BaseCpuInfo& addFeature(uint32_t feature) {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    _features[feature / kFeaturesPerUInt32] |= (1U << (feature % kFeaturesPerUInt32));
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Detect number of cores (or sum of all cores of all processors).
  static ASMJIT_API uint32_t detectNumberOfCores();

  //! Get host cpu.
  static ASMJIT_API const BaseCpuInfo* getHost();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Size of the structure in bytes.
  uint32_t _size;

  //! Cpu short vendor string.
  char _vendorString[16];
  //! Cpu long vendor string (brand).
  char _brandString[64];

  //! Cpu vendor id, see `asmjit::kCpuVendor`.
  uint32_t _vendorId;
  //! Cpu family ID.
  uint32_t _family;
  //! Cpu model ID.
  uint32_t _model;
  //! Cpu stepping.
  uint32_t _stepping;
  //! Cpu cores count (or sum of all CPU cores of all processors).
  uint32_t _coresCount;

  //! Cpu features bitfield.
  uint32_t _features[4];
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CPUINFO_H
