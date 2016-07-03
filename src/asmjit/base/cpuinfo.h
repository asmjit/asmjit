// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CPUINFO_H
#define _ASMJIT_BASE_CPUINFO_H

// [Dependencies]
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
class CpuInfo {
 public:
  // --------------------------------------------------------------------------
  // [Vendor]
  // --------------------------------------------------------------------------

  //! CPU vendor ID.
  ASMJIT_ENUM(Vendor) {
    kVendorNone  = 0,                    //!< Generic or unknown.
    kVendorIntel = 1,                    //!< Intel vendor.
    kVendorAMD   = 2,                    //!< AMD vendor.
    kVendorVIA   = 3                     //!< VIA vendor.
  };

  // --------------------------------------------------------------------------
  // [ArmFeatures]
  // --------------------------------------------------------------------------

  //! ARM/ARM64 CPU features.
  ASMJIT_ENUM(ArmFeatures) {
    kArmFeatureV6,                       //!< ARMv6 instruction set.
    kArmFeatureV7,                       //!< ARMv7 instruction set.
    kArmFeatureV8,                       //!< ARMv8 instruction set.
    kArmFeatureTHUMB,                    //!< CPU provides THUMB v1 instruction set (ARM only).
    kArmFeatureTHUMB2,                   //!< CPU provides THUMB v2 instruction set (ARM only).
    kArmFeatureVFP2,                     //!< CPU provides VFPv2 instruction set.
    kArmFeatureVFP3,                     //!< CPU provides VFPv3 instruction set.
    kArmFeatureVFP4,                     //!< CPU provides VFPv4 instruction set.
    kArmFeatureVFP_D32,                  //!< CPU provides 32 VFP-D (64-bit) registers.
    kArmFeatureNEON,                     //!< CPU provides NEON instruction set.
    kArmFeatureDSP,                      //!< CPU provides DSP extensions.
    kArmFeatureIDIV,                     //!< CPU provides hardware support for SDIV and UDIV.
    kArmFeatureAES,                      //!< CPU provides AES instructions (ARM64 only).
    kArmFeatureCRC32,                    //!< CPU provides CRC32 instructions (ARM64 only).
    kArmFeaturePMULL,                    //!< CPU provides PMULL instructions (ARM64 only).
    kArmFeatureSHA1,                     //!< CPU provides SHA1 instructions (ARM64 only).
    kArmFeatureSHA256,                   //!< CPU provides SHA256 instructions (ARM64 only).
    kArmFeatureAtomics64,                //!< CPU provides 64-bit load/store atomics (ARM64 only).

    kArmFeaturesCount                    //!< Count of ARM/ARM64 CPU features.
  };

  // --------------------------------------------------------------------------
  // [X86Features]
  // --------------------------------------------------------------------------

  //! X86/X64 CPU features.
  ASMJIT_ENUM(X86Features) {
    kX86FeatureNX = 0,                   //!< CPU has Not-Execute-Bit.
    kX86FeatureMT,                       //!< CPU has multi-threading.
    kX86FeatureRDTSC,                    //!< CPU has RDTSC.
    kX86FeatureRDTSCP,                   //!< CPU has RDTSCP.
    kX86FeatureCMOV,                     //!< CPU has CMOV.
    kX86FeatureCMPXCHG8B,                //!< CPU has CMPXCHG8B.
    kX86FeatureCMPXCHG16B,               //!< CPU has CMPXCHG16B (x64).
    kX86FeatureCLFLUSH,                  //!< CPU has CLFUSH.
    kX86FeatureCLFLUSH_OPT,              //!< CPU has CLFUSH (optimized).
    kX86FeatureCLWB,                     //!< CPU has CLWB.
    kX86FeaturePCOMMIT,                  //!< CPU has PCOMMIT.
    kX86FeaturePREFETCH,                 //!< CPU has PREFETCH.
    kX86FeaturePREFETCHWT1,              //!< CPU has PREFETCHWT1.
    kX86FeatureLAHF_SAHF,                //!< CPU has LAHF/SAHF.
    kX86FeatureFXSR,                     //!< CPU has FXSAVE/FXRSTOR.
    kX86FeatureFXSR_OPT,                 //!< CPU has FXSAVE/FXRSTOR (optimized).
    kX86FeatureMMX,                      //!< CPU has MMX.
    kX86FeatureMMX2,                     //!< CPU has extended MMX.
    kX86Feature3DNOW,                    //!< CPU has 3dNow!
    kX86Feature3DNOW2,                   //!< CPU has enhanced 3dNow!
    kX86FeatureSSE,                      //!< CPU has SSE.
    kX86FeatureSSE2,                     //!< CPU has SSE2.
    kX86FeatureSSE3,                     //!< CPU has SSE3.
    kX86FeatureSSSE3,                    //!< CPU has SSSE3.
    kX86FeatureSSE4A,                    //!< CPU has SSE4.A.
    kX86FeatureSSE4_1,                   //!< CPU has SSE4.1.
    kX86FeatureSSE4_2,                   //!< CPU has SSE4.2.
    kX86FeatureMSSE,                     //!< CPU has Misaligned SSE (MSSE).
    kX86FeatureMONITOR,                  //!< CPU has MONITOR and MWAIT.
    kX86FeatureMOVBE,                    //!< CPU has MOVBE.
    kX86FeaturePOPCNT,                   //!< CPU has POPCNT.
    kX86FeatureLZCNT,                    //!< CPU has LZCNT.
    kX86FeatureAESNI,                    //!< CPU has AESNI.
    kX86FeaturePCLMULQDQ,                //!< CPU has PCLMULQDQ.
    kX86FeatureRDRAND,                   //!< CPU has RDRAND.
    kX86FeatureRDSEED,                   //!< CPU has RDSEED.
    kX86FeatureSMAP,                     //!< CPU has SMAP (supervisor-mode access prevention).
    kX86FeatureSMEP,                     //!< CPU has SMEP (supervisor-mode execution prevention).
    kX86FeatureSHA,                      //!< CPU has SHA-1 and SHA-256.
    kX86FeatureXSAVE,                    //!< CPU has XSAVE support - XSAVE/XRSTOR, XSETBV/XGETBV, and XCR0.
    kX86FeatureXSAVE_OS,                 //!< OS has enabled XSAVE, you can call XGETBV to get value of XCR0.
    kX86FeatureAVX,                      //!< CPU has AVX.
    kX86FeatureAVX2,                     //!< CPU has AVX2.
    kX86FeatureF16C,                     //!< CPU has F16C.
    kX86FeatureFMA3,                     //!< CPU has FMA3.
    kX86FeatureFMA4,                     //!< CPU has FMA4.
    kX86FeatureXOP,                      //!< CPU has XOP.
    kX86FeatureBMI,                      //!< CPU has BMI (bit manipulation instructions #1).
    kX86FeatureBMI2,                     //!< CPU has BMI2 (bit manipulation instructions #2).
    kX86FeatureADX,                      //!< CPU has ADX (multi-precision add-carry instruction extensions).
    kX86FeatureTBM,                      //!< CPU has TBM (trailing bit manipulation).
    kX86FeatureMPX,                      //!< CPU has MPX (memory protection extensions).
    kX86FeatureHLE,                      //!< CPU has HLE.
    kX86FeatureRTM,                      //!< CPU has RTM.
    kX86FeatureERMS,                     //!< CPU has ERMS (enhanced REP MOVSB/STOSB).
    kX86FeatureFSGSBASE,                 //!< CPU has FSGSBASE.
    kX86FeatureAVX512F,                  //!< CPU has AVX-512F (foundation).
    kX86FeatureAVX512CD,                 //!< CPU has AVX-512CD (conflict detection).
    kX86FeatureAVX512PF,                 //!< CPU has AVX-512PF (prefetch instructions).
    kX86FeatureAVX512ER,                 //!< CPU has AVX-512ER (exponential and reciprocal instructions).
    kX86FeatureAVX512DQ,                 //!< CPU has AVX-512DQ (DWORD/QWORD).
    kX86FeatureAVX512BW,                 //!< CPU has AVX-512BW (BYTE/WORD).
    kX86FeatureAVX512VL,                 //!< CPU has AVX VL (vector length extensions).
    kX86FeatureAVX512IFMA,               //!< CPU has AVX IFMA (integer fused multiply add using 52-bit precision).
    kX86FeatureAVX512VBMI,               //!< CPU has AVX VBMI (vector byte manipulation instructions).

    kX86FeaturesCount                    //!< Count of X86/X64 CPU features.
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
    uint32_t _processorType;             //!< Processor type.
    uint32_t _brandIndex;                //!< Brand index.
    uint32_t _flushCacheLineSize;        //!< Flush cache line size (in bytes).
    uint32_t _maxLogicalProcessors;      //!< Maximum number of addressable IDs for logical processors.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE CpuInfo() noexcept { reset(); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() noexcept { ::memset(this, 0, sizeof(CpuInfo)); }

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

  //! Get the host CPU information.
  static ASMJIT_API const CpuInfo& getHost() noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! CPU vendor string.
  char _vendorString[16];
  //! CPU brand string.
  char _brandString[64];

  //! CPU architecture, see \ref Arch.
  uint8_t _arch;
  //! \internal
  uint8_t _reserved[3];
  //! CPU vendor id, see \ref CpuVendor.
  uint32_t _vendorId;
  //! CPU family ID.
  uint32_t _family;
  //! CPU model ID.
  uint32_t _model;
  //! CPU stepping.
  uint32_t _stepping;

  //! Number of hardware threads.
  uint32_t _hwThreadsCount;

  //! CPU features (bit-array).
  uint32_t _features[8];

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
