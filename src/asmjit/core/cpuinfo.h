// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_CPUINFO_H
#define _ASMJIT_CORE_CPUINFO_H

#include "../core/arch.h"
#include "../core/features.h"
#include "../core/globals.h"
#include "../core/string.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::CpuInfo]
// ============================================================================

//! CPU information.
class CpuInfo {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline CpuInfo() noexcept { reset(); }
  inline CpuInfo(const CpuInfo& other) noexcept = default;
  inline explicit CpuInfo(Globals::NoInit_) noexcept
    : _archInfo(Globals::NoInit), _features(Globals::NoInit) {};

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initializes CpuInfo to the given architecture, see `ArchInfo`.
  inline void initArch(uint32_t archId, uint32_t archMode = 0) noexcept {
    _archInfo.init(archId, archMode);
  }

  inline void reset() noexcept { ::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the CPU architecture information.
  inline const ArchInfo& archInfo() const noexcept { return _archInfo; }
  //! Gets the CPU architecture id, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _archInfo.archId(); }
  //! Gets the CPU architecture sub-id, see `ArchInfo::SubId`.
  inline uint32_t archSubId() const noexcept { return _archInfo.archSubId(); }

  //! Gets the CPU family ID.
  inline uint32_t familyId() const noexcept { return _familyId; }
  //! Gets the CPU model ID.
  inline uint32_t modelId() const noexcept { return _modelId; }
  //! Gets the CPU brand id.
  inline uint32_t brandId() const noexcept { return _brandId; }
  //! Gets the CPU stepping.
  inline uint32_t stepping() const noexcept { return _stepping; }
  //! Gets the processor type.
  inline uint32_t processorType() const noexcept { return _processorType; }
  //! Gets the number of maximum logical processors.
  inline uint32_t maxLogicalProcessors() const noexcept { return _maxLogicalProcessors; }

  //! Gets the size of a cache line flush.
  inline uint32_t cacheLineSize() const noexcept { return _cacheLineSize; }
  //! Gets number of hardware threads available.
  inline uint32_t hwThreadCount() const noexcept { return _hwThreadCount; }

  //! Gets the CPU vendor.
  inline const char* vendor() const noexcept { return _vendor.str; }
  //! Gets whether the CPU vendor is equal to `s`.
  inline bool isVendor(const char* s) const noexcept { return _vendor.eq(s); }

  //! Gets the CPU brand string.
  inline const char* brand() const noexcept { return _brand.str; }

  //! Gets all CPU features as `BaseFeatures`, cast to your arch-specific class
  //! if needed.
  template<typename T = BaseFeatures>
  inline const T& features() const noexcept { return _features.as<T>(); }

  //! Gets whether the CPU has the given `feature`.
  inline bool hasFeature(uint32_t featureId) const noexcept { return _features.has(featureId); }
  //! Adds the given CPU `feature` to the list of this CpuInfo features.
  inline CpuInfo& addFeature(uint32_t featureId) noexcept { _features.add(featureId); return *this; }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Gets the host CPU information.
  ASMJIT_API static const CpuInfo& host() noexcept;

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline CpuInfo& operator=(const CpuInfo& other) noexcept = default;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! CPU architecture information.
  ArchInfo _archInfo;
  //! CPU family ID.
  uint32_t _familyId;
  //! CPU model ID.
  uint32_t _modelId;
  //! CPU brand ID.
  uint32_t _brandId;
  //! CPU stepping.
  uint32_t _stepping;
  //! Processor type.
  uint32_t _processorType;
  //! Maximum number of addressable IDs for logical processors.
  uint32_t _maxLogicalProcessors;
  //! Cache line size (in bytes).
  uint32_t _cacheLineSize;
  //! Number of hardware threads.
  uint32_t _hwThreadCount;

  //! CPU vendor string.
  FixedString<16> _vendor;
  //! CPU brand string.
  FixedString<64> _brand;
  //! CPU features.
  BaseFeatures _features;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_CPUINFO_H
