// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_CPUINFO_H
#define _ASMJIT_CORE_CPUINFO_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/features.h"
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

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initialize CpuInfo to the given architecture, see `ArchInfo`.
  inline void initArch(uint32_t archId, uint32_t archMode = 0) noexcept {
    _archInfo.init(archId, archMode);
  }

  inline void reset() noexcept { ::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get generic architecture information.
  inline const ArchInfo& archInfo() const noexcept { return _archInfo; }
  //! Get CPU architecture type, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _archInfo.archId(); }
  //! Get CPU architecture sub-type, see `ArchInfo::SubType`.
  inline uint32_t archSubId() const noexcept { return _archInfo.archSubId(); }

  //! Get CPU family ID.
  inline uint32_t familyId() const noexcept { return _familyId; }
  //! Get CPU model ID.
  inline uint32_t modelId() const noexcept { return _modelId; }
  //! Get CPU brand id.
  inline uint32_t brandId() const noexcept { return _brandId; }
  //! Get CPU stepping.
  inline uint32_t stepping() const noexcept { return _stepping; }
  //! Get processor type.
  inline uint32_t processorType() const noexcept { return _processorType; }
  //! Get the number of maximum logical processors.
  inline uint32_t maxLogicalProcessors() const noexcept { return _maxLogicalProcessors; }

  //! Get the size of a cache line flush.
  inline uint32_t cacheLineSize() const noexcept { return _cacheLineSize; }
  //! Get number of hardware threads available.
  inline uint32_t hwThreadCount() const noexcept { return _hwThreadCount; }

  //! Get CPU vendor.
  inline const char* vendor() const noexcept { return _vendor.str; }
  //! Check whether the CPU vendor is equal to string `s`.
  inline bool isVendor(const char* s) const noexcept { return _vendor.eq(s); }

  //! Get CPU brand string.
  inline const char* brand() const noexcept { return _brand.str; }

  //! Get all CPU features as `BaseFeatures`, cast to your arch-specific class if needed.
  template<typename T = BaseFeatures>
  inline const T& features() const noexcept { return _features.as<T>(); }

  //! Get whether CPU has a `feature`.
  inline bool hasFeature(uint32_t featureId) const noexcept { return _features.has(featureId); }
  //! Add a CPU `feature`.
  inline CpuInfo& addFeature(uint32_t featureId) noexcept { _features.add(featureId); return *this; }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Get the host CPU information.
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

// [Guard]
#endif // _ASMJIT_CORE_CPUINFO_H
