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

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::CpuInfo]
// ============================================================================

//! CPU information.
struct CpuInfo {
  // --------------------------------------------------------------------------
  // [Vendor]
  // --------------------------------------------------------------------------

  //! CPU vendor ID.
  ASMJIT_ENUM(Vendor) {
    //! Generic or unknown.
    kVendorNone = 0,

    //! Intel vendor.
    kVendorIntel = 1,
    //! AMD vendor.
    kVendorAMD = 2,
    //! VIA vendor.
    kVendorVIA = 3
  };

  // --------------------------------------------------------------------------
  // [ArmFeatures]
  // --------------------------------------------------------------------------

  //! ARM/ARM64 CPU features.
  ASMJIT_ENUM(ArmFeatures) {
    //! ARMv6 instruction set.
    kArmFeatureV6,
    //! ARMv7 instruction set.
    kArmFeatureV7,
    //! ARMv8 instruction set.
    kArmFeatureV8,

    //! CPU provides THUMB v1 instruction set (ARM only).
    kArmFeatureTHUMB,
    //! CPU provides THUMB v2 instruction set (ARM only).
    kArmFeatureTHUMB2,

    //! CPU provides VFPv2 instruction set.
    kArmFeatureVFP2,
    //! CPU provides VFPv3 instruction set.
    kArmFeatureVFP3,
    //! CPU provides VFPv4 instruction set.
    kArmFeatureVFP4,
    //! CPU provides 32 VFP-D (64-bit) registers.
    kArmFeatureVFP_D32,

    //! CPU provides NEON instruction set.
    kArmFeatureNEON,

    //! CPU provides DSP extensions.
    kArmFeatureDSP,
    //! CPU provides hardware support for SDIV and UDIV.
    kArmFeatureIDIV,

    //! CPU provides AES instructions (ARM64 only).
    kArmFeatureAES,
    //! CPU provides CRC32 instructions (ARM64 only).
    kArmFeatureCRC32,
    //! CPU provides PMULL instructions (ARM64 only).
    kArmFeaturePMULL,
    //! CPU provides SHA1 instructions (ARM64 only).
    kArmFeatureSHA1,
    //! CPU provides SHA256 instructions (ARM64 only).
    kArmFeatureSHA256,
    //! CPU provides 64-bit load/store atomics (ARM64 only).
    kArmFeatureAtomics64,

    //! Count of ARM/ARM64 CPU features.
    kArmFeaturesCount
  };

  // --------------------------------------------------------------------------
  // [X86Features]
  // --------------------------------------------------------------------------

  //! X86/X64 CPU features.
  ASMJIT_ENUM(X86Features) {
    //! Cpu has Not-Execute-Bit.
    kX86FeatureNX = 0,
    //! Cpu has multithreading.
    kX86FeatureMT,
    //! Cpu has RDTSC.
    kX86FeatureRDTSC,
    //! Cpu has RDTSCP.
    kX86FeatureRDTSCP,
    //! Cpu has CMOV.
    kX86FeatureCMOV,
    //! Cpu has CMPXCHG8B.
    kX86FeatureCMPXCHG8B,
    //! Cpu has CMPXCHG16B (X64).
    kX86FeatureCMPXCHG16B,
    //! Cpu has CLFUSH.
    kX86FeatureCLFLUSH,
    //! Cpu has CLFUSH (Optimized).
    kX86FeatureCLFLUSH_OPT,
    //! Cpu has PREFETCH.
    kX86FeaturePREFETCH,
    //! Cpu has PREFETCHWT1.
    kX86FeaturePREFETCHWT1,
    //! Cpu has LAHF/SAHF.
    kX86FeatureLAHF_SAHF,
    //! Cpu has FXSAVE/FXRSTOR.
    kX86FeatureFXSR,
    //! Cpu has FXSAVE/FXRSTOR (Optimized).
    kX86FeatureFXSR_OPT,
    //! Cpu has MMX.
    kX86FeatureMMX,
    //! Cpu has extended MMX.
    kX86FeatureMMX2,
    //! Cpu has 3dNow!
    kX86Feature3DNOW,
    //! Cpu has enchanced 3dNow!
    kX86Feature3DNOW2,
    //! Cpu has SSE.
    kX86FeatureSSE,
    //! Cpu has SSE2.
    kX86FeatureSSE2,
    //! Cpu has SSE3.
    kX86FeatureSSE3,
    //! Cpu has SSSE3.
    kX86FeatureSSSE3,
    //! Cpu has SSE4.A.
    kX86FeatureSSE4A,
    //! Cpu has SSE4.1.
    kX86FeatureSSE4_1,
    //! Cpu has SSE4.2.
    kX86FeatureSSE4_2,
    //! Cpu has Misaligned SSE (MSSE).
    kX86FeatureMSSE,
    //! Cpu has MONITOR and MWAIT.
    kX86FeatureMONITOR,
    //! Cpu has MOVBE.
    kX86FeatureMOVBE,
    //! Cpu has POPCNT.
    kX86FeaturePOPCNT,
    //! Cpu has LZCNT.
    kX86FeatureLZCNT,
    //! Cpu has AESNI.
    kX86FeatureAESNI,
    //! Cpu has PCLMULQDQ.
    kX86FeaturePCLMULQDQ,
    //! Cpu has RDRAND.
    kX86FeatureRDRAND,
    //! Cpu has RDSEED.
    kX86FeatureRDSEED,
    //! Cpu has SHA-1 and SHA-256.
    kX86FeatureSHA,
    //! Cpu has XSAVE support - XSAVE/XRSTOR, XSETBV/XGETBV, and XCR0.
    kX86FeatureXSAVE,
    //! OS has enabled XSAVE, you can call XGETBV to get value of XCR0.
    kX86FeatureXSAVE_OS,
    //! Cpu has AVX.
    kX86FeatureAVX,
    //! Cpu has AVX2.
    kX86FeatureAVX2,
    //! Cpu has F16C.
    kX86FeatureF16C,
    //! Cpu has FMA3.
    kX86FeatureFMA3,
    //! Cpu has FMA4.
    kX86FeatureFMA4,
    //! Cpu has XOP.
    kX86FeatureXOP,
    //! Cpu has BMI.
    kX86FeatureBMI,
    //! Cpu has BMI2.
    kX86FeatureBMI2,
    //! Cpu has HLE.
    kX86FeatureHLE,
    //! Cpu has RTM.
    kX86FeatureRTM,
    //! Cpu has ADX.
    kX86FeatureADX,
    //! Cpu has MPX (Memory Protection Extensions).
    kX86FeatureMPX,
    //! Cpu has FSGSBASE.
    kX86FeatureFSGSBASE,
    //! Cpu has optimized REP MOVSB/STOSB.
    kX86FeatureMOVSBSTOSB_OPT,

    //! Cpu has AVX-512F (Foundation).
    kX86FeatureAVX512F,
    //! Cpu has AVX-512CD (Conflict Detection).
    kX86FeatureAVX512CD,
    //! Cpu has AVX-512PF (Prefetch Instructions).
    kX86FeatureAVX512PF,
    //! Cpu has AVX-512ER (Exponential and Reciprocal Instructions).
    kX86FeatureAVX512ER,
    //! Cpu has AVX-512DQ (DWord/QWord).
    kX86FeatureAVX512DQ,
    //! Cpu has AVX-512BW (Byte/Word).
    kX86FeatureAVX512BW,
    //! Cpu has AVX VL (Vector Length Excensions).
    kX86FeatureAVX512VL,

    //! Count of X86/X64 CPU features.
    kX86FeaturesCount
  };

  // --------------------------------------------------------------------------
  // [Other]
  // --------------------------------------------------------------------------

  //! \internal
  enum {
    kFeaturesPerUInt32 = static_cast<int>(sizeof(uint32_t)) * 8
  };

  // --------------------------------------------------------------------------
  // [ArmInfo]
  // --------------------------------------------------------------------------

  struct ArmData {
  };

  // --------------------------------------------------------------------------
  // [X86Info]
  // --------------------------------------------------------------------------

  struct X86Data {
    //! Processor type.
    uint32_t _processorType;
    //! Brand index.
    uint32_t _brandIndex;
    //! Flush cache line size in bytes.
    uint32_t _flushCacheLineSize;
    //! Maximum number of addressable IDs for logical processors.
    uint32_t _maxLogicalProcessors;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE CpuInfo() noexcept { reset(); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() noexcept {
    ::memset(this, 0, sizeof(CpuInfo));
  }

  // --------------------------------------------------------------------------
  // [Detect]
  // --------------------------------------------------------------------------

  ASMJIT_API void detect() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get CPU architecture, see \Arch.
  ASMJIT_INLINE uint32_t getArch() const noexcept { return _arch; }
  //! Set CPU architecture, see \Arch.
  ASMJIT_INLINE void setArch(uint32_t arch) noexcept { _arch = static_cast<uint8_t>(arch); }

  //! Get CPU vendor string.
  ASMJIT_INLINE const char* getVendorString() const noexcept { return _vendorString; }
  //! Get CPU brand string.
  ASMJIT_INLINE const char* getBrandString() const noexcept { return _brandString; }

  //! Get CPU vendor ID.
  ASMJIT_INLINE uint32_t getVendorId() const noexcept { return _vendorId; }
  //! Get CPU family ID.
  ASMJIT_INLINE uint32_t getFamily() const noexcept { return _family; }
  //! Get CPU model ID.
  ASMJIT_INLINE uint32_t getModel() const noexcept { return _model; }
  //! Get CPU stepping.
  ASMJIT_INLINE uint32_t getStepping() const noexcept { return _stepping; }

  //! Get number of hardware threads available.
  ASMJIT_INLINE uint32_t getHwThreadsCount() const noexcept {
    return _hwThreadsCount;
  }

  //! Get whether CPU has a `feature`.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const noexcept {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    uint32_t pos = feature / kFeaturesPerUInt32;
    uint32_t bit = feature % kFeaturesPerUInt32;

    return static_cast<bool>((_features[pos] >> bit) & 0x1);
  }

  //! Add a CPU `feature`.
  ASMJIT_INLINE CpuInfo& addFeature(uint32_t feature) noexcept {
    ASMJIT_ASSERT(feature < sizeof(_features) * 8);

    uint32_t pos = feature / kFeaturesPerUInt32;
    uint32_t bit = feature % kFeaturesPerUInt32;

    _features[pos] |= static_cast<uint32_t>(1) << bit;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Accessors - ARM]
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // [Accessors - X86]
  // --------------------------------------------------------------------------

  //! Get processor type.
  ASMJIT_INLINE uint32_t getX86ProcessorType() const noexcept {
    return _x86Data._processorType;
  }

  //! Get brand index.
  ASMJIT_INLINE uint32_t getX86BrandIndex() const noexcept {
    return _x86Data._brandIndex;
  }

  //! Get flush cache line size.
  ASMJIT_INLINE uint32_t getX86FlushCacheLineSize() const noexcept {
    return _x86Data._flushCacheLineSize;
  }

  //! Get maximum logical processors count.
  ASMJIT_INLINE uint32_t getX86MaxLogicalProcessors() const noexcept {
    return _x86Data._maxLogicalProcessors;
  }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Get host cpu.
  static ASMJIT_API const CpuInfo& getHost() noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Cpu vendor string.
  char _vendorString[16];
  //! Cpu brand string.
  char _brandString[64];

  //! CPU architecture, see \ref Arch.
  uint8_t _arch;
  //! \internal
  uint8_t _reserved[3];
  //! Cpu vendor id, see \ref CpuVendor.
  uint32_t _vendorId;
  //! Cpu family ID.
  uint32_t _family;
  //! Cpu model ID.
  uint32_t _model;
  //! Cpu stepping.
  uint32_t _stepping;

  //! Number of hardware threads.
  uint32_t _hwThreadsCount;

  //! Cpu features bitfield.
  uint32_t _features[4];

  // Architecture specific data.
  union {
    ArmData _armData;
    X86Data _x86Data;
  };
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CPUINFO_H
