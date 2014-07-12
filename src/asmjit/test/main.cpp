// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include "../asmjit.h"

using namespace asmjit;

// ============================================================================
// [DumpCpu]
// ============================================================================

struct DumpCpuFeature {
  uint32_t feature;
  const char* name;
};

static void dumpCpuFeatures(const CpuInfo* cpuInfo, const DumpCpuFeature* data, size_t count) {
  for (size_t i = 0; i < count; i++)
    if (cpuInfo->hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu(void) {
  const CpuInfo* cpu = CpuInfo::getHost();

  INFO("Host CPU Info:");
  INFO("  Vendor string         : %s", cpu->getVendorString());
  INFO("  Brand string          : %s", cpu->getBrandString());
  INFO("  Family                : %u", cpu->getFamily());
  INFO("  Model                 : %u", cpu->getModel());
  INFO("  Stepping              : %u", cpu->getStepping());
  INFO("  HW-Threads Count      : %u", cpu->getHwThreadsCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
  const X86CpuInfo* x86Cpu = static_cast<const X86CpuInfo*>(cpu);

  static const DumpCpuFeature x86FeaturesList[] = {
    { kX86CpuFeatureMultithreading     , "Multithreading"      },
    { kX86CpuFeatureExecuteDisableBit  , "Execute-Disable Bit" },
    { kX86CpuFeatureRdtsc              , "Rdtsc"               },
    { kX86CpuFeatureRdtscp             , "Rdtscp"              },
    { kX86CpuFeatureCmov               , "Cmov"                },
    { kX86CpuFeatureCmpXchg8B          , "Cmpxchg8b"           },
    { kX86CpuFeatureCmpXchg16B         , "Cmpxchg16b"          },
    { kX86CpuFeatureClflush            , "Clflush"             },
    { kX86CpuFeaturePrefetch           , "Prefetch"            },
    { kX86CpuFeatureLahfSahf           , "Lahf/Sahf"           },
    { kX86CpuFeatureFxsr               , "Fxsave/Fxrstor"      },
    { kX86CpuFeatureFfxsr              , "Fxsave/Fxrstor Opt." },
    { kX86CpuFeatureMmx                , "Mmx"                 },
    { kX86CpuFeatureMmxExt             , "MmxExt"              },
    { kX86CpuFeature3dNow              , "3dnow"               },
    { kX86CpuFeature3dNowExt           , "3dnowExt"            },
    { kX86CpuFeatureSse                , "Sse"                 },
    { kX86CpuFeatureSse2               , "Sse2"                },
    { kX86CpuFeatureSse3               , "Sse3"                },
    { kX86CpuFeatureSsse3              , "Ssse3"               },
    { kX86CpuFeatureSse4A              , "Sse4a"               },
    { kX86CpuFeatureSse41              , "Sse4.1"              },
    { kX86CpuFeatureSse42              , "Sse4.2"              },
    { kX86CpuFeatureMsse               , "Misaligned SSE"      },
    { kX86CpuFeatureMonitorMWait       , "Monitor/MWait"       },
    { kX86CpuFeatureMovbe              , "Movbe"               },
    { kX86CpuFeaturePopcnt             , "Popcnt"              },
    { kX86CpuFeatureLzcnt              , "Lzcnt"               },
    { kX86CpuFeatureAesni              , "AesNI"               },
    { kX86CpuFeaturePclmulqdq          , "Pclmulqdq"           },
    { kX86CpuFeatureRdrand             , "Rdrand"              },
    { kX86CpuFeatureAvx                , "Avx"                 },
    { kX86CpuFeatureAvx2               , "Avx2"                },
    { kX86CpuFeatureF16C               , "F16C"                },
    { kX86CpuFeatureFma3               , "Fma3"                },
    { kX86CpuFeatureFma4               , "Fma4"                },
    { kX86CpuFeatureXop                , "Xop"                 },
    { kX86CpuFeatureBmi                , "Bmi"                 },
    { kX86CpuFeatureBmi2               , "Bmi2"                },
    { kX86CpuFeatureHle                , "Hle"                 },
    { kX86CpuFeatureRtm                , "Rtm"                 },
    { kX86CpuFeatureFsGsBase           , "FsGsBase"            },
    { kX86CpuFeatureRepMovsbStosbExt   , "RepMovsbStosbExt"    }
  };

  INFO("Host CPU Info (X86/X64):");
  INFO("  Processor Type        : %u", x86Cpu->getProcessorType());
  INFO("  Brand Index           : %u", x86Cpu->getBrandIndex());
  INFO("  CL Flush Cache Line   : %u", x86Cpu->getFlushCacheLineSize());
  INFO("  Max logical Processors: %u", x86Cpu->getMaxLogicalProcessors());
  INFO("");

  INFO("Host CPU Features (X86/X64):");
  dumpCpuFeatures(x86Cpu, x86FeaturesList, ASMJIT_ARRAY_SIZE(x86FeaturesList));
  INFO("");
#endif // ASMJIT_HOST || ASMJIT_HOST_X64
}

// ============================================================================
// [DumpSizeOf]
// ============================================================================

#define DUMP_TYPE(_Type_) \
  INFO("  %-31s: %u", #_Type_, static_cast<uint32_t>(sizeof(_Type_)))

static void dumpSizeOf(void) {
  INFO("SizeOf Types:");
  DUMP_TYPE(int8_t);
  DUMP_TYPE(int16_t);
  DUMP_TYPE(int32_t);
  DUMP_TYPE(int64_t);
  DUMP_TYPE(int);
  DUMP_TYPE(long);
  DUMP_TYPE(size_t);
  DUMP_TYPE(intptr_t);
  DUMP_TYPE(float);
  DUMP_TYPE(double);
  DUMP_TYPE(void*);
  INFO("");

  INFO("SizeOf Base:");
  DUMP_TYPE(asmjit::CodeGen);
  DUMP_TYPE(asmjit::ConstPool);
  DUMP_TYPE(asmjit::Runtime);
  DUMP_TYPE(asmjit::Zone);
  DUMP_TYPE(asmjit::Ptr);
  DUMP_TYPE(asmjit::SignedPtr);
  INFO("");

  INFO("SizeOf Operand:");
  DUMP_TYPE(asmjit::Operand);
  DUMP_TYPE(asmjit::Reg);
  DUMP_TYPE(asmjit::Var);
  DUMP_TYPE(asmjit::BaseMem);
  DUMP_TYPE(asmjit::Imm);
  DUMP_TYPE(asmjit::Label);
  INFO("");

  INFO("SizeOf Assembler:");
  DUMP_TYPE(asmjit::Assembler);
  DUMP_TYPE(asmjit::LabelData);
  DUMP_TYPE(asmjit::RelocData);
  INFO("");

#if !defined(ASMJIT_DISABLE_COMPILER)
  INFO("SizeOf Compiler:");
  DUMP_TYPE(asmjit::Compiler);
  DUMP_TYPE(asmjit::Node);
  DUMP_TYPE(asmjit::AlignNode);
  DUMP_TYPE(asmjit::CallNode);
  DUMP_TYPE(asmjit::CommentNode);
  DUMP_TYPE(asmjit::EmbedNode);
  DUMP_TYPE(asmjit::FuncNode);
  DUMP_TYPE(asmjit::EndNode);
  DUMP_TYPE(asmjit::InstNode);
  DUMP_TYPE(asmjit::JumpNode);
  DUMP_TYPE(asmjit::TargetNode);
  DUMP_TYPE(asmjit::FuncDecl);
  DUMP_TYPE(asmjit::FuncInOut);
  DUMP_TYPE(asmjit::FuncPrototype);
  DUMP_TYPE(asmjit::VarAttr);
  DUMP_TYPE(asmjit::VarData);
  DUMP_TYPE(asmjit::VarMap);
  DUMP_TYPE(asmjit::VarState);
  INFO("");
#endif // !ASMJIT_DISABLE_COMPILER

  // --------------------------------------------------------------------------
  // [X86/X64]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
  INFO("SizeOf X86/X64:");
  DUMP_TYPE(asmjit::X86Assembler);
#if !defined(ASMJIT_DISABLE_COMPILER)
  DUMP_TYPE(asmjit::X86Compiler);
  DUMP_TYPE(asmjit::X86CallNode);
  DUMP_TYPE(asmjit::X86FuncNode);
  DUMP_TYPE(asmjit::X86FuncDecl);
  DUMP_TYPE(asmjit::X86InstInfo);
  DUMP_TYPE(asmjit::X86VarMap);
  DUMP_TYPE(asmjit::X86VarInfo);
  DUMP_TYPE(asmjit::X86VarState);
#endif // !ASMJIT_DISABLE_COMPILER
  INFO("");
#endif // ASMJIT_BUILD_X86
}

#undef DUMP_TYPE

// ============================================================================
// [Main]
// ============================================================================

static void onBeforeRun(void) {
  dumpCpu();
  dumpSizeOf();
}

int main(int argc, const char* argv[]) {
  INFO("AsmJit Unit-Test\n\n");
  return BrokenAPI::run(argc, argv, onBeforeRun);
}
