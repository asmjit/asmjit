// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86CPUINFO_H
#define _ASMJIT_X86_X86CPUINFO_H

// [Dependencies - AsmJit]
#include "../base/cpuinfo.h"
#include "../base/defs.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [asmjit::x86x64::kCpuFeature]
// ============================================================================

//! @brief X86 CPU features.
ASMJIT_ENUM(kCpuFeature) {
  //! @brief Cpu has multithreading.
  kCpuFeatureMultithreading = 1,
  //! @brief Cpu has execute disable bit.
  kCpuFeatureExecuteDisableBit,
  //! @brief Cpu has RDTSC.
  kCpuFeatureRdtsc,
  //! @brief Cpu has RDTSCP.
  kCpuFeatureRdtscp,
  //! @brief Cpu has CMOV.
  kCpuFeatureCmov,
  //! @brief Cpu has CMPXCHG8B.
  kCpuFeatureCmpXchg8B,
  //! @brief Cpu has CMPXCHG16B (x64).
  kCpuFeatureCmpXchg16B,
  //! @brief Cpu has CLFUSH.
  kCpuFeatureClflush,
  //! @brief Cpu has PREFETCH.
  kCpuFeaturePrefetch,
  //! @brief Cpu has LAHF/SAHF.
  kCpuFeatureLahfSahf,
  //! @brief Cpu has FXSAVE/FXRSTOR.
  kCpuFeatureFxsr,
  //! @brief Cpu has FXSAVE/FXRSTOR optimizations.
  kCpuFeatureFfxsr,
  //! @brief Cpu has MMX.
  kCpuFeatureMmx,
  //! @brief Cpu has extended MMX.
  kCpuFeatureMmxExt,
  //! @brief Cpu has 3dNow!
  kCpuFeature3dNow,
  //! @brief Cpu has enchanced 3dNow!
  kCpuFeature3dNowExt,
  //! @brief Cpu has SSE.
  kCpuFeatureSse,
  //! @brief Cpu has SSE2.
  kCpuFeatureSse2,
  //! @brief Cpu has SSE3.
  kCpuFeatureSse3,
  //! @brief Cpu has Supplemental SSE3 (SSSE3).
  kCpuFeatureSsse3,
  //! @brief Cpu has SSE4.A.
  kCpuFeatureSse4A,
  //! @brief Cpu has SSE4.1.
  kCpuFeatureSse41,
  //! @brief Cpu has SSE4.2.
  kCpuFeatureSse42,
  //! @brief Cpu has Misaligned SSE (MSSE).
  kCpuFeatureMsse,
  //! @brief Cpu has MONITOR and MWAIT.
  kCpuFeatureMonitorMWait,
  //! @brief Cpu has MOVBE.
  kCpuFeatureMovbe,
  //! @brief Cpu has POPCNT.
  kCpuFeaturePopcnt,
  //! @brief Cpu has LZCNT.
  kCpuFeatureLzcnt,
  //! @brief Cpu has AESNI.
  kCpuFeatureAesni,
  //! @brief Cpu has PCLMULQDQ.
  kCpuFeaturePclmulqdq,
  //! @brief Cpu has RDRAND.
  kCpuFeatureRdrand,
  //! @brief Cpu has AVX.
  kCpuFeatureAvx,
  //! @brief Cpu has AVX2.
  kCpuFeatureAvx2,
  //! @brief Cpu has F16C.
  kCpuFeatureF16C,
  //! @brief Cpu has FMA3.
  kCpuFeatureFma3,
  //! @brief Cpu has FMA4.
  kCpuFeatureFma4,
  //! @brief Cpu has XOP.
  kCpuFeatureXop,
  //! @brief Cpu has BMI.
  kCpuFeatureBmi,
  //! @brief Cpu has BMI2.
  kCpuFeatureBmi2,
  //! @brief Cpu has HLE.
  kCpuFeatureHle,
  //! @brief Cpu has RTM.
  kCpuFeatureRtm,
  //! @brief Cpu has FSGSBASE.
  kCpuFeatureFsGsBase,
  //! @brief Cpu has enhanced REP MOVSB/STOSB.
  kCpuFeatureRepMovsbStosbExt,

  //! @brief Count of X86/X64 Cpu features.
  kCpuFeatureCount
};

// ============================================================================
// [asmjit::x86x64::CpuId]
// ============================================================================

//! @brief X86/X64 cpuid output.
union CpuId {
  //! @brief EAX/EBX/ECX/EDX output.
  uint32_t i[4];

  struct {
    //! @brief EAX output.
    uint32_t eax;
    //! @brief EBX output.
    uint32_t ebx;
    //! @brief ECX output.
    uint32_t ecx;
    //! @brief EDX output.
    uint32_t edx;
  };
};

// ============================================================================
// [asmjit::x86x64::CpuInfo]
// ============================================================================

struct CpuInfo : public BaseCpuInfo {
  ASMJIT_NO_COPY(CpuInfo)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE CpuInfo(uint32_t size = sizeof(CpuInfo)) :
    BaseCpuInfo(size) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get processor type.
  ASMJIT_INLINE uint32_t getProcessorType() const {
    return _processorType;
  }

  //! @brief Get brand index.
  ASMJIT_INLINE uint32_t getBrandIndex() const {
    return _brandIndex;
  }

  //! @brief Get flush cache line size.
  ASMJIT_INLINE uint32_t getFlushCacheLineSize() const {
    return _flushCacheLineSize;
  }

  //! @brief Get maximum logical processors count.
  ASMJIT_INLINE uint32_t getMaxLogicalProcessors() const {
    return _maxLogicalProcessors;
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! @brief Get global instance of @ref X86CpuInfo.
  static ASMJIT_INLINE const CpuInfo* getHost() {
    return static_cast<const CpuInfo*>(BaseCpuInfo::getHost());
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Processor type.
  uint32_t _processorType;
  //! @brief Brand index.
  uint32_t _brandIndex;
  //! @brief Flush cache line size in bytes.
  uint32_t _flushCacheLineSize;
  //! @brief Maximum number of addressable IDs for logical processors.
  uint32_t _maxLogicalProcessors;
};

// ============================================================================
// [asmjit::x86x64::hostCpuId / hostCpuDetect]
// ============================================================================

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
//! @brief Get the result of calling CPUID instruction.
ASMJIT_API void hostCpuId(uint32_t inEax, uint32_t inEcx, CpuId* result);
//! @brief Detect host CPU.
ASMJIT_API void hostCpuDetect(CpuInfo* cpuInfo);
#endif // ASMJIT_HOST_X86 || ASMJIT_HOST_X64

//! @}

} // x86x64 namespace
} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86CPUINFO_H
