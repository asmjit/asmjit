// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include <asmjit/asmjit.h>

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

struct CpuFeature {
  uint32_t feature;
  const char* description;
};

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
static const CpuFeature x86x64Features[] = {
  { x86x64::kCpuFeatureMultithreading     , "Multithreading"      },
  { x86x64::kCpuFeatureExecuteDisableBit  , "Execute-Disable Bit" },
  { x86x64::kCpuFeatureRdtsc              , "Rdtsc"               },
  { x86x64::kCpuFeatureRdtscp             , "Rdtscp"              },
  { x86x64::kCpuFeatureCmov               , "Cmov"                },
  { x86x64::kCpuFeatureCmpXchg8B          , "Cmpxchg8b"           },
  { x86x64::kCpuFeatureCmpXchg16B         , "Cmpxchg16b"          },
  { x86x64::kCpuFeatureClflush            , "Clflush"             },
  { x86x64::kCpuFeaturePrefetch           , "Prefetch"            },
  { x86x64::kCpuFeatureLahfSahf           , "Lahf/Sahf"           },
  { x86x64::kCpuFeatureFxsr               , "Fxsave/Fxrstor"      },
  { x86x64::kCpuFeatureFfxsr              , "Fxsave/Fxrstor Opt." },
  { x86x64::kCpuFeatureMmx                , "Mmx"                 },
  { x86x64::kCpuFeatureMmxExt             , "MmxExt"              },
  { x86x64::kCpuFeature3dNow              , "3dnow"               },
  { x86x64::kCpuFeature3dNowExt           , "3dnowExt"            },
  { x86x64::kCpuFeatureSse                , "Sse"                 },
  { x86x64::kCpuFeatureSse2               , "Sse2"                },
  { x86x64::kCpuFeatureSse3               , "Sse3"                },
  { x86x64::kCpuFeatureSsse3              , "Ssse3"               },
  { x86x64::kCpuFeatureSse4A              , "Sse4a"               },
  { x86x64::kCpuFeatureSse41              , "Sse4.1"              },
  { x86x64::kCpuFeatureSse42              , "Sse4.2"              },
  { x86x64::kCpuFeatureMsse               , "Misaligned SSE"      },
  { x86x64::kCpuFeatureMonitorMWait       , "Monitor/MWait"       },
  { x86x64::kCpuFeatureMovbe              , "Movbe"               },
  { x86x64::kCpuFeaturePopcnt             , "Popcnt"              },
  { x86x64::kCpuFeatureLzcnt              , "Lzcnt"               },
  { x86x64::kCpuFeatureAesni              , "AesNI"               },
  { x86x64::kCpuFeaturePclmulqdq          , "Pclmulqdq"           },
  { x86x64::kCpuFeatureRdrand             , "Rdrand"              },
  { x86x64::kCpuFeatureAvx                , "Avx"                 },
  { x86x64::kCpuFeatureAvx2               , "Avx2"                },
  { x86x64::kCpuFeatureF16C               , "F16C"                },
  { x86x64::kCpuFeatureFma3               , "Fma3"                },
  { x86x64::kCpuFeatureFma4               , "Fma4"                },
  { x86x64::kCpuFeatureXop                , "Xop"                 },
  { x86x64::kCpuFeatureBmi                , "Bmi"                 },
  { x86x64::kCpuFeatureBmi2               , "Bmi2"                },
  { x86x64::kCpuFeatureHle                , "Hle"                 },
  { x86x64::kCpuFeatureRtm                , "Rtm"                 },
  { x86x64::kCpuFeatureFsGsBase           , "FsGsBase"            },
  { x86x64::kCpuFeatureRepMovsbStosbExt   , "RepMovsbStosbExt"    }
};
#endif // ASMJIT_HOST || ASMJIT_HOST_X64

static void printFeatures(const char* prefix, const BaseCpuInfo* cpuInfo, const CpuFeature* data) {
  for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(x86x64Features); i++) {
    if (cpuInfo->hasFeature(data[i].feature)) {
      printf("%s%s\n", prefix, data[i].description);
    }
  }
}

int main(int argc, char* argv[]) {
  const BaseCpuInfo* cpuInfo_ = BaseCpuInfo::getHost();

  // --------------------------------------------------------------------------
  // [Core Features]
  // --------------------------------------------------------------------------

  printf("Host CPU\n");
  printf("========\n");

  printf("\nBasic info\n");
  printf("  Vendor string         : %s\n", cpuInfo_->getVendorString());
  printf("  Brand string          : %s\n", cpuInfo_->getBrandString());
  printf("  Family                : %u\n", cpuInfo_->getFamily());
  printf("  Model                 : %u\n", cpuInfo_->getModel());
  printf("  Stepping              : %u\n", cpuInfo_->getStepping());
  printf("  Cores Count           : %u\n", cpuInfo_->getCoresCount());

  // --------------------------------------------------------------------------
  // [X86 Features]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
  const x86x64::CpuInfo* cpuInfo = static_cast<const x86x64::CpuInfo*>(cpuInfo_);

  printf("\nX86/X64 Extended Info:\n");
  printf("  Processor Type        : %u\n", cpuInfo->getProcessorType());
  printf("  Brand Index           : %u\n", cpuInfo->getBrandIndex());
  printf("  CL Flush Cache Line   : %u\n", cpuInfo->getFlushCacheLineSize());
  printf("  Max logical Processors: %u\n", cpuInfo->getMaxLogicalProcessors());

  printf("\nX86/X64 Features:\n");
  printFeatures("  ", cpuInfo, x86x64Features);
#endif // ASMJIT_HOST || ASMJIT_HOST_X64

  return 0;
}
