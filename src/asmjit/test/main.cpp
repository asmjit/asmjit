// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - MiniUnit]
#include "./test.h"
#include "../asmjit.h"

using namespace asmjit;

// ============================================================================
// [DumpCpu]
// ============================================================================

struct DumpCpuFeature {
  uint32_t feature;
  const char* name;
};

static void dumpCpuFeatures(const BaseCpuInfo* cpuInfo, const DumpCpuFeature* data, size_t count) {
  for (size_t i = 0; i < count; i++)
    if (cpuInfo->hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu() {
  const BaseCpuInfo* cpuInfo_ = BaseCpuInfo::getHost();

  INFO("Host CPU Info:");
  INFO("  Vendor string         : %s", cpuInfo_->getVendorString());
  INFO("  Brand string          : %s", cpuInfo_->getBrandString());
  INFO("  Family                : %u", cpuInfo_->getFamily());
  INFO("  Model                 : %u", cpuInfo_->getModel());
  INFO("  Stepping              : %u", cpuInfo_->getStepping());
  INFO("  Cores Count           : %u", cpuInfo_->getCoresCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
  const x86x64::CpuInfo* cpuInfo = static_cast<const x86x64::CpuInfo*>(cpuInfo_);

  static const DumpCpuFeature featuresList[] = {
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

  INFO("Host CPU Info (X86/X64):");
  INFO("  Processor Type        : %u", cpuInfo->getProcessorType());
  INFO("  Brand Index           : %u", cpuInfo->getBrandIndex());
  INFO("  CL Flush Cache Line   : %u", cpuInfo->getFlushCacheLineSize());
  INFO("  Max logical Processors: %u", cpuInfo->getMaxLogicalProcessors());
  INFO("");

  INFO("Host CPU Features (X86/X64):");
  dumpCpuFeatures(cpuInfo, featuresList, ASMJIT_ARRAY_SIZE(featuresList));
  INFO("");
#endif // ASMJIT_HOST || ASMJIT_HOST_X64
}

// ============================================================================
// [DumpSizeOf]
// ============================================================================

#define DUMP_SIZE(_Type_) \
  INFO("  %-31s: %u", #_Type_, static_cast<uint32_t>(sizeof(_Type_)))

static void dumpSizeOf() {
  INFO("SizeOf Types:");
  DUMP_SIZE(int8_t);
  DUMP_SIZE(int16_t);
  DUMP_SIZE(int32_t);
  DUMP_SIZE(int64_t);
  DUMP_SIZE(int);
  DUMP_SIZE(long);
  DUMP_SIZE(size_t);
  DUMP_SIZE(intptr_t);
  DUMP_SIZE(float);
  DUMP_SIZE(double);
  DUMP_SIZE(void*);
  DUMP_SIZE(asmjit::Ptr);
  DUMP_SIZE(asmjit::SignedPtr);
  INFO("");

  INFO("SizeOf Base:");
  DUMP_SIZE(asmjit::CodeGen);
  DUMP_SIZE(asmjit::ConstPool);
  DUMP_SIZE(asmjit::Runtime);
  DUMP_SIZE(asmjit::Zone);
  INFO("");

  INFO("SizeOf Operand:");
  DUMP_SIZE(asmjit::Operand);
  DUMP_SIZE(asmjit::BaseReg);
  DUMP_SIZE(asmjit::BaseVar);
  DUMP_SIZE(asmjit::BaseMem);
  DUMP_SIZE(asmjit::Imm);
  DUMP_SIZE(asmjit::Label);
  INFO("");

  INFO("SizeOf Assembler:");
  DUMP_SIZE(asmjit::BaseAssembler);
  DUMP_SIZE(asmjit::LabelData);
  DUMP_SIZE(asmjit::RelocData);
  INFO("");

#if !defined(ASMJIT_DISABLE_COMPILER)
  INFO("SizeOf Compiler:");
  DUMP_SIZE(asmjit::BaseCompiler);
  DUMP_SIZE(asmjit::Node);
  DUMP_SIZE(asmjit::AlignNode);
  DUMP_SIZE(asmjit::CallNode);
  DUMP_SIZE(asmjit::CommentNode);
  DUMP_SIZE(asmjit::EmbedNode);
  DUMP_SIZE(asmjit::FuncNode);
  DUMP_SIZE(asmjit::EndNode);
  DUMP_SIZE(asmjit::InstNode);
  DUMP_SIZE(asmjit::JumpNode);
  DUMP_SIZE(asmjit::TargetNode);
  DUMP_SIZE(asmjit::FuncDecl);
  DUMP_SIZE(asmjit::FuncInOut);
  DUMP_SIZE(asmjit::FuncPrototype);
  DUMP_SIZE(asmjit::VarAttr);
  DUMP_SIZE(asmjit::VarData);
  DUMP_SIZE(asmjit::BaseVarInst);
  DUMP_SIZE(asmjit::BaseVarState);
  INFO("");
#endif // !ASMJIT_DISABLE_COMPILER

  // --------------------------------------------------------------------------
  // [X86/X64]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
  INFO("SizeOf X86/X64:");
  DUMP_SIZE(asmjit::x86x64::X86X64Assembler);
  DUMP_SIZE(asmjit::x86x64::X86X64Compiler);
  DUMP_SIZE(asmjit::x86x64::X86X64CallNode);
  DUMP_SIZE(asmjit::x86x64::X86X64FuncNode);
  DUMP_SIZE(asmjit::x86x64::X86X64FuncDecl);
  DUMP_SIZE(asmjit::x86x64::VarInst);
  DUMP_SIZE(asmjit::x86x64::VarState);
  DUMP_SIZE(asmjit::x86x64::InstInfo);
  DUMP_SIZE(asmjit::x86x64::VarInfo);
  INFO("");
#endif // ASMJIT_BUILD_X86
}

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, const char* argv[]) {
  if (MiniUnit::init(argc, argv)) {
    dumpCpu();
    dumpSizeOf();

    MiniUnit::run();
  }

  return 0;
}
