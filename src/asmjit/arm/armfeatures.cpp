// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#if defined(ASMJIT_BUILD_ARM) && ASMJIT_ARCH_ARM

#include "../core/cpuinfo.h"
#include "../core/support.h"
#include "../arm/armfeatures.h"

// Required by `getauxval()` on Linux.
#if defined(__linux__)
  #include <sys/auxv.h>
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

// ============================================================================
// [asmjit::arm::Features - Detect - Base]
// ============================================================================

static inline void populateBaseFeatures(CpuInfo& cpu) noexcept {
  #if ASMJIT_ARCH_ARM == 32
  cpu._archInfo.init(ArchInfo::kIdA32);
  #else
  cpu._archInfo.init(ArchInfo::kIdA64);

  // AArch64 is based on ARMv8 and later.
  cpu.addFeature(Features::kARMv6);
  cpu.addFeature(Features::kARMv7);
  cpu.addFeature(Features::kARMv8);

  // AArch64 comes with these features by default.
  cpu.addFeature(Features::kVFPv2);
  cpu.addFeature(Features::kVFPv3);
  cpu.addFeature(Features::kVFPv4);
  cpu.addFeature(Features::kEDSP);
  cpu.addFeature(Features::kASIMD);
  cpu.addFeature(Features::kIDIVA);
  cpu.addFeature(Features::kIDIVT);
  #endif
}

// ============================================================================
// [asmjit::arm::Features - Detect - Windows]
// ============================================================================

#if defined(_WIN32)
struct WinPFPMapping {
  uint8_t featureId;
  uint8_t pfpFeatureId;
};

static void detectPFPFeatures(CpuInfo& cpu, const WinPFPMapping* mapping, size_t size) noexcept {
  for (size_t i = 0; i < size; i++)
    if (::IsProcessorFeaturePresent(mapping[i].pfpFeatureId))
      cpu.addFeature(mapping[i].featureId);
}

//! Detect ARM CPU features on Windows.
//!
//! The detection is based on `IsProcessorFeaturePresent()` API call.
ASMJIT_FAVOR_SIZE void detectCpu(CpuInfo& cpu) noexcept {
  populateBaseFeatures(cpu);

  Features& features = cpu._features.as<Features>();

  // Win32 for ARM requires ARMv7 with DSP extensions, VFPv3, and uses THUMBv2 by default.
  #if ASMJIT_ARCH_ARM == 32
  features.add(Features::kARMv6);
  features.add(Features::kARMv7);
  features.add(Features::kEDSP);
  features.add(Features::kVFPv2);
  features.add(Features::kVFPv3);
  features.add(Features::kTHUMB);
  features.add(Features::kTHUMBv2);
  #endif

  // Windows for ARM requires ASIMD.
  features.add(Features::kASIMD);

  // Detect additional CPU features by calling `IsProcessorFeaturePresent()`.
  static const WinPFPMapping mapping[] = {
    { Features::kVFPv4    , PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE  },
    { Features::kVFP_D32  , PF_ARM_VFP_32_REGISTERS_AVAILABLE   },
    { Features::kIDIVT    , PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE },
    { Features::kATOMIC64 , PF_ARM_64BIT_LOADSTORE_ATOMIC       }
  };
  detectPFPFeatures(cpu, mapping, ASMJIT_ARRAY_SIZE(mapping));
}
#endif

// ============================================================================
// [asmjit::arm::Features - Detect - Linux]
// ============================================================================

#if defined(__linux__)
struct LinuxHWCapMapping {
  uint8_t featureId;
  uint8_t hwCapBit;
};

static void detectHWCaps(CpuInfo& cpu, unsigned long type, const LinuxHWCapMapping* mapping, size_t size) noexcept {
  unsigned long mask = getauxval(type);
  for (size_t i = 0; i < size; i++)
    if (Support::bitTest(mask, mapping[i].hwCapBit))
      cpu.addFeature(mapping[i].featureId);
}

//! Detect ARM CPU features on Linux.
//!
//! The detection is based on `getauxval()`.
ASMJIT_FAVOR_SIZE void detectCpu(CpuInfo& cpu) noexcept {
  populateBaseFeatures(cpu);
  Features& features = cpu._features.as<Features>();

  #if ASMJIT_ARCH_ARM == 32
  // `AT_HWCAP` provides ARMv7 (and less) related flags.
  static const LinuxHWCapMapping hwCapMapping[] = {
    { Features::kVFPv2   , 6  }, // HWCAP_VFP
    { Features::kEDSP    , 7  }, // HWCAP_EDSP
    { Features::kASIMD   , 12 }, // HWCAP_NEON
    { Features::kVFPv3   , 13 }, // HWCAP_VFPv3
    { Features::kVFPv4   , 16 }, // HWCAP_VFPv4
    { Features::kIDIVA   , 17 }, // HWCAP_IDIVA
    { Features::kIDIVT   , 18 }, // HWCAP_IDIVT
    { Features::kVFP_D32 , 19 }  // HWCAP_VFPD32
  };
  detectHWCaps(cpu, AT_HWCAP, hwCapMapping, ASMJIT_ARRAY_SIZE(hwCapMapping));

  // VFPv3 implies VFPv2.
  if (features.hasVFPv3())
    features.add(Features::kVFPv2);

  // VFPv2 implies ARMv6.
  if (features.hasVFPv2())
    features.add(Features::kARMv6);

  // VFPv3|ASIMD implies ARMv7.
  if (features.hasVFPv3() || features.hasASIMD())
    features.add(Features::kARMv7);

  // `AT_HWCAP2` provides ARMv8+ related flags.
  static const LinuxHWCapMapping hwCap2Mapping[] = {
    { Features::kAES     ,  0 }, // HWCAP2_AES
    { Features::kPMULL   ,  1 }, // HWCAP2_PMULL
    { Features::kSHA1    ,  2 }, // HWCAP2_SHA1
    { Features::kSHA256  ,  3 }, // HWCAP2_SHA2
    { Features::kCRC32   ,  4 }  // HWCAP2_CRC32
  };
  detectHWCaps(cpu, AT_HWCAP2, hwCap2Mapping, ASMJIT_ARRAY_SIZE(hwCap2Mapping));

  if (features.hasAES() || features.hasCRC32() || features.hasPMULL() || features.hasSHA1() || features.hasSHA256())
    features.add(Features::kARMv8);
  #else
  // `AT_HWCAP` provides ARMv8+ related flags.
  static const LinuxHWCapMapping hwCapMapping[] = {
    { Features::kASIMD   ,  1 }, // HWCAP_ASIMD
    { Features::kAES     ,  3 }, // HWCAP_AES
    { Features::kCRC32   ,  7 }, // HWCAP_CRC32
    { Features::kPMULL   ,  4 }, // HWCAP_PMULL
    { Features::kSHA1    ,  5 }, // HWCAP_SHA1
    { Features::kSHA256  ,  6 }, // HWCAP_SHA2
    { Features::kATOMIC64,  8 }  // HWCAP_ATOMICS
  };
  detectHWCaps(cpu, AT_HWCAP, hwCapMapping, ASMJIT_ARRAY_SIZE(hwCapMapping));

  // `AT_HWCAP2` is not used at the moment.
  #endif
}
#endif

// ============================================================================
// [asmjit::arm::Features - Detect - Unknown]
// ============================================================================

#if !defined(_WIN32) && !defined(__linux__)
  #error "[asmjit] arm::detectCpu() - Unsupported OS."
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_ARM && ASMJIT_ARCH_ARM
