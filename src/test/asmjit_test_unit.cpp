// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include "../asmjit/asmjit.h"

// ============================================================================
// [DumpCpu]
// ============================================================================

struct DumpCpuFeature {
  uint32_t feature;
  const char* name;
};

static void dumpCpuFeatures(const asmjit::CpuInfo* cpuInfo, const DumpCpuFeature* data, size_t count) {
  for (size_t i = 0; i < count; i++)
    if (cpuInfo->hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu(void) {
  const asmjit::CpuInfo* cpu = asmjit::CpuInfo::getHost();

  INFO("Host CPU Info:");
  INFO("  Vendor string              : %s", cpu->getVendorString());
  INFO("  Brand string               : %s", cpu->getBrandString());
  INFO("  Family                     : %u", cpu->getFamily());
  INFO("  Model                      : %u", cpu->getModel());
  INFO("  Stepping                   : %u", cpu->getStepping());
  INFO("  HW-Threads Count           : %u", cpu->getHwThreadsCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_ARCH_X86) || defined(ASMJIT_ARCH_X64)
  const asmjit::X86CpuInfo* x86Cpu = static_cast<const asmjit::X86CpuInfo*>(cpu);

  static const DumpCpuFeature x86FeaturesList[] = {
    { asmjit::kX86CpuFeatureNX            , "NX (Non-Execute Bit)"  },
    { asmjit::kX86CpuFeatureMT            , "MT (Multi-Threading)"  },
    { asmjit::kX86CpuFeatureRDTSC         , "RDTSC"                 },
    { asmjit::kX86CpuFeatureRDTSCP        , "RDTSCP"                },
    { asmjit::kX86CpuFeatureCMOV          , "CMOV"                  },
    { asmjit::kX86CpuFeatureCMPXCHG8B     , "CMPXCHG8B"             },
    { asmjit::kX86CpuFeatureCMPXCHG16B    , "CMPXCHG16B"            },
    { asmjit::kX86CpuFeatureCLFLUSH       , "CLFLUSH"               },
    { asmjit::kX86CpuFeatureCLFLUSHOpt    , "CLFLUSH (Opt)"         },
    { asmjit::kX86CpuFeaturePREFETCH      , "PREFETCH"              },
    { asmjit::kX86CpuFeaturePREFETCHWT1   , "PREFETCHWT1"           },
    { asmjit::kX86CpuFeatureLahfSahf      , "LAHF/SAHF"             },
    { asmjit::kX86CpuFeatureFXSR          , "FXSR"                  },
    { asmjit::kX86CpuFeatureFXSROpt       , "FXSR (Opt)"            },
    { asmjit::kX86CpuFeatureMMX           , "MMX"                   },
    { asmjit::kX86CpuFeatureMMX2          , "MMX2"                  },
    { asmjit::kX86CpuFeature3DNOW         , "3DNOW"                 },
    { asmjit::kX86CpuFeature3DNOW2        , "3DNOW2"                },
    { asmjit::kX86CpuFeatureSSE           , "SSE"                   },
    { asmjit::kX86CpuFeatureSSE2          , "SSE2"                  },
    { asmjit::kX86CpuFeatureSSE3          , "SSE3"                  },
    { asmjit::kX86CpuFeatureSSSE3         , "SSSE3"                 },
    { asmjit::kX86CpuFeatureSSE4A         , "SSE4A"                 },
    { asmjit::kX86CpuFeatureSSE4_1        , "SSE4.1"                },
    { asmjit::kX86CpuFeatureSSE4_2        , "SSE4.2"                },
    { asmjit::kX86CpuFeatureMSSE          , "Misaligned SSE"        },
    { asmjit::kX86CpuFeatureMONITOR       , "MONITOR/MWAIT"         },
    { asmjit::kX86CpuFeatureMOVBE         , "MOVBE"                 },
    { asmjit::kX86CpuFeaturePOPCNT        , "POPCNT"                },
    { asmjit::kX86CpuFeatureLZCNT         , "LZCNT"                 },
    { asmjit::kX86CpuFeatureAESNI         , "AESNI"                 },
    { asmjit::kX86CpuFeaturePCLMULQDQ     , "PCLMULQDQ"             },
    { asmjit::kX86CpuFeatureRDRAND        , "RDRAND"                },
    { asmjit::kX86CpuFeatureRDSEED        , "RDSEED"                },
    { asmjit::kX86CpuFeatureSHA           , "SHA"                   },
    { asmjit::kX86CpuFeatureXSave         , "XSAVE"                 },
    { asmjit::kX86CpuFeatureXSaveOS       , "XSAVE (OS)"            },
    { asmjit::kX86CpuFeatureAVX           , "AVX"                   },
    { asmjit::kX86CpuFeatureAVX2          , "AVX2"                  },
    { asmjit::kX86CpuFeatureF16C          , "F16C"                  },
    { asmjit::kX86CpuFeatureFMA3          , "FMA3"                  },
    { asmjit::kX86CpuFeatureFMA4          , "FMA4"                  },
    { asmjit::kX86CpuFeatureXOP           , "XOP"                   },
    { asmjit::kX86CpuFeatureBMI           , "BMI"                   },
    { asmjit::kX86CpuFeatureBMI2          , "BMI2"                  },
    { asmjit::kX86CpuFeatureHLE           , "HLE"                   },
    { asmjit::kX86CpuFeatureRTM           , "RTM"                   },
    { asmjit::kX86CpuFeatureADX           , "ADX"                   },
    { asmjit::kX86CpuFeatureMPX           , "MPX"                   },
    { asmjit::kX86CpuFeatureFSGSBase      , "FS/GS Base"            },
    { asmjit::kX86CpuFeatureMOVSBSTOSBOpt , "REP MOVSB/STOSB (Opt)" },
    { asmjit::kX86CpuFeatureAVX512F       , "AVX512F"               },
    { asmjit::kX86CpuFeatureAVX512CD      , "AVX512CD"              },
    { asmjit::kX86CpuFeatureAVX512PF      , "AVX512PF"              },
    { asmjit::kX86CpuFeatureAVX512ER      , "AVX512ER"              },
    { asmjit::kX86CpuFeatureAVX512DQ      , "AVX512DQ"              },
    { asmjit::kX86CpuFeatureAVX512BW      , "AVX512BW"              },
    { asmjit::kX86CpuFeatureAVX512VL      , "AVX512VL"              }
  };

  INFO("Host CPU Info (X86/X64):");
  INFO("  Processor Type             : %u", x86Cpu->getProcessorType());
  INFO("  Brand Index                : %u", x86Cpu->getBrandIndex());
  INFO("  CL Flush Cache Line        : %u", x86Cpu->getFlushCacheLineSize());
  INFO("  Max logical Processors     : %u", x86Cpu->getMaxLogicalProcessors());
  INFO("");

  INFO("Host CPU Features (X86/X64):");
  dumpCpuFeatures(x86Cpu, x86FeaturesList, ASMJIT_ARRAY_SIZE(x86FeaturesList));
  INFO("");
#endif
}

// ============================================================================
// [DumpSizeOf]
// ============================================================================

#define DUMP_TYPE(_Type_) \
  INFO("  %-27s: %u", #_Type_, static_cast<uint32_t>(sizeof(_Type_)))

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
  DUMP_TYPE(asmjit::X86InstInfo);
  DUMP_TYPE(asmjit::X86InstExtendedInfo);

#if !defined(ASMJIT_DISABLE_COMPILER)
  DUMP_TYPE(asmjit::X86Compiler);
  DUMP_TYPE(asmjit::X86CallNode);
  DUMP_TYPE(asmjit::X86FuncNode);
  DUMP_TYPE(asmjit::X86FuncDecl);
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
