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

static void dumpCpuFeatures(const asmjit::CpuInfo& cpu, const DumpCpuFeature* data, size_t count) {
  for (size_t i = 0; i < count; i++)
    if (cpu.hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu(void) {
  const asmjit::CpuInfo& cpu = asmjit::CpuInfo::getHost();

  INFO("Host CPU:");
  INFO("  Vendor string              : %s", cpu.getVendorString());
  INFO("  Brand string               : %s", cpu.getBrandString());
  INFO("  Family                     : %u", cpu.getFamily());
  INFO("  Model                      : %u", cpu.getModel());
  INFO("  Stepping                   : %u", cpu.getStepping());
  INFO("  HW-Threads Count           : %u", cpu.getHwThreadsCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [ARM / ARM64]
  // --------------------------------------------------------------------------

#if ASMJIT_ARCH_ARM32 || ASMJIT_ARCH_ARM64
  static const DumpCpuFeature armFeaturesList[] = {
    { asmjit::CpuInfo::kArmFeatureV6            , "ARMv6"                 },
    { asmjit::CpuInfo::kArmFeatureV7            , "ARMv7"                 },
    { asmjit::CpuInfo::kArmFeatureV8            , "ARMv8"                 },
    { asmjit::CpuInfo::kArmFeatureTHUMB         , "THUMB"                 },
    { asmjit::CpuInfo::kArmFeatureTHUMB2        , "THUMBv2"               },
    { asmjit::CpuInfo::kArmFeatureVFP2          , "VFPv2"                 },
    { asmjit::CpuInfo::kArmFeatureVFP3          , "VFPv3"                 },
    { asmjit::CpuInfo::kArmFeatureVFP4          , "VFPv4"                 },
    { asmjit::CpuInfo::kArmFeatureVFP_D32       , "VFP D32"               },
    { asmjit::CpuInfo::kArmFeatureNEON          , "NEON"                  },
    { asmjit::CpuInfo::kArmFeatureDSP           , "DSP"                   },
    { asmjit::CpuInfo::kArmFeatureIDIV          , "IDIV"                  },
    { asmjit::CpuInfo::kArmFeatureAES           , "AES"                   },
    { asmjit::CpuInfo::kArmFeatureCRC32         , "CRC32"                 },
    { asmjit::CpuInfo::kArmFeatureSHA1          , "SHA1"                  },
    { asmjit::CpuInfo::kArmFeatureSHA256        , "SHA256"                },
    { asmjit::CpuInfo::kArmFeatureAtomics64     , "64-bit atomics"        }
  };

  INFO("ARM Features:");
  dumpCpuFeatures(cpu, armFeaturesList, ASMJIT_ARRAY_SIZE(armFeaturesList));
  INFO("");
#endif

  // --------------------------------------------------------------------------
  // [X86 / X64]
  // --------------------------------------------------------------------------

#if ASMJIT_ARCH_X86 || ASMJIT_ARCH_X64
  static const DumpCpuFeature x86FeaturesList[] = {
    { asmjit::CpuInfo::kX86FeatureNX            , "NX (Non-Execute Bit)"  },
    { asmjit::CpuInfo::kX86FeatureMT            , "MT (Multi-Threading)"  },
    { asmjit::CpuInfo::kX86FeatureRDTSC         , "RDTSC"                 },
    { asmjit::CpuInfo::kX86FeatureRDTSCP        , "RDTSCP"                },
    { asmjit::CpuInfo::kX86FeatureCMOV          , "CMOV"                  },
    { asmjit::CpuInfo::kX86FeatureCMPXCHG8B     , "CMPXCHG8B"             },
    { asmjit::CpuInfo::kX86FeatureCMPXCHG16B    , "CMPXCHG16B"            },
    { asmjit::CpuInfo::kX86FeatureCLFLUSH       , "CLFLUSH"               },
    { asmjit::CpuInfo::kX86FeatureCLFLUSH_OPT   , "CLFLUSH (Opt)"         },
    { asmjit::CpuInfo::kX86FeaturePREFETCH      , "PREFETCH"              },
    { asmjit::CpuInfo::kX86FeaturePREFETCHWT1   , "PREFETCHWT1"           },
    { asmjit::CpuInfo::kX86FeatureLAHF_SAHF     , "LAHF/SAHF"             },
    { asmjit::CpuInfo::kX86FeatureFXSR          , "FXSR"                  },
    { asmjit::CpuInfo::kX86FeatureFXSR_OPT      , "FXSR (Opt)"            },
    { asmjit::CpuInfo::kX86FeatureMMX           , "MMX"                   },
    { asmjit::CpuInfo::kX86FeatureMMX2          , "MMX2"                  },
    { asmjit::CpuInfo::kX86Feature3DNOW         , "3DNOW"                 },
    { asmjit::CpuInfo::kX86Feature3DNOW2        , "3DNOW2"                },
    { asmjit::CpuInfo::kX86FeatureSSE           , "SSE"                   },
    { asmjit::CpuInfo::kX86FeatureSSE2          , "SSE2"                  },
    { asmjit::CpuInfo::kX86FeatureSSE3          , "SSE3"                  },
    { asmjit::CpuInfo::kX86FeatureSSSE3         , "SSSE3"                 },
    { asmjit::CpuInfo::kX86FeatureSSE4A         , "SSE4A"                 },
    { asmjit::CpuInfo::kX86FeatureSSE4_1        , "SSE4.1"                },
    { asmjit::CpuInfo::kX86FeatureSSE4_2        , "SSE4.2"                },
    { asmjit::CpuInfo::kX86FeatureMSSE          , "Misaligned SSE"        },
    { asmjit::CpuInfo::kX86FeatureMONITOR       , "MONITOR/MWAIT"         },
    { asmjit::CpuInfo::kX86FeatureMOVBE         , "MOVBE"                 },
    { asmjit::CpuInfo::kX86FeaturePOPCNT        , "POPCNT"                },
    { asmjit::CpuInfo::kX86FeatureLZCNT         , "LZCNT"                 },
    { asmjit::CpuInfo::kX86FeatureAESNI         , "AESNI"                 },
    { asmjit::CpuInfo::kX86FeaturePCLMULQDQ     , "PCLMULQDQ"             },
    { asmjit::CpuInfo::kX86FeatureRDRAND        , "RDRAND"                },
    { asmjit::CpuInfo::kX86FeatureRDSEED        , "RDSEED"                },
    { asmjit::CpuInfo::kX86FeatureSHA           , "SHA"                   },
    { asmjit::CpuInfo::kX86FeatureXSAVE         , "XSAVE"                 },
    { asmjit::CpuInfo::kX86FeatureXSAVE_OS      , "XSAVE (OS)"            },
    { asmjit::CpuInfo::kX86FeatureAVX           , "AVX"                   },
    { asmjit::CpuInfo::kX86FeatureAVX2          , "AVX2"                  },
    { asmjit::CpuInfo::kX86FeatureF16C          , "F16C"                  },
    { asmjit::CpuInfo::kX86FeatureFMA3          , "FMA3"                  },
    { asmjit::CpuInfo::kX86FeatureFMA4          , "FMA4"                  },
    { asmjit::CpuInfo::kX86FeatureXOP           , "XOP"                   },
    { asmjit::CpuInfo::kX86FeatureBMI           , "BMI"                   },
    { asmjit::CpuInfo::kX86FeatureBMI2          , "BMI2"                  },
    { asmjit::CpuInfo::kX86FeatureHLE           , "HLE"                   },
    { asmjit::CpuInfo::kX86FeatureRTM           , "RTM"                   },
    { asmjit::CpuInfo::kX86FeatureADX           , "ADX"                   },
    { asmjit::CpuInfo::kX86FeatureMPX           , "MPX"                   },
    { asmjit::CpuInfo::kX86FeatureFSGSBASE      , "FS/GS Base"            },
    { asmjit::CpuInfo::kX86FeatureMOVSBSTOSB_OPT, "REP MOVSB/STOSB (Opt)" },
    { asmjit::CpuInfo::kX86FeatureAVX512F       , "AVX512F"               },
    { asmjit::CpuInfo::kX86FeatureAVX512CD      , "AVX512CD"              },
    { asmjit::CpuInfo::kX86FeatureAVX512PF      , "AVX512PF"              },
    { asmjit::CpuInfo::kX86FeatureAVX512ER      , "AVX512ER"              },
    { asmjit::CpuInfo::kX86FeatureAVX512DQ      , "AVX512DQ"              },
    { asmjit::CpuInfo::kX86FeatureAVX512BW      , "AVX512BW"              },
    { asmjit::CpuInfo::kX86FeatureAVX512VL      , "AVX512VL"              }
  };

  INFO("X86 Specific:");
  INFO("  Processor Type             : %u", cpu.getX86ProcessorType());
  INFO("  Brand Index                : %u", cpu.getX86BrandIndex());
  INFO("  CL Flush Cache Line        : %u", cpu.getX86FlushCacheLineSize());
  INFO("  Max logical Processors     : %u", cpu.getX86MaxLogicalProcessors());
  INFO("");

  INFO("X86 Features:");
  dumpCpuFeatures(cpu, x86FeaturesList, ASMJIT_ARRAY_SIZE(x86FeaturesList));
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
  DUMP_TYPE(asmjit::Ptr);
  DUMP_TYPE(asmjit::SignedPtr);
  INFO("");

  INFO("SizeOf Base:");
  DUMP_TYPE(asmjit::Assembler);
  DUMP_TYPE(asmjit::ConstPool);
  DUMP_TYPE(asmjit::LabelData);
  DUMP_TYPE(asmjit::RelocData);
  DUMP_TYPE(asmjit::Runtime);
  DUMP_TYPE(asmjit::Zone);
  INFO("");

  INFO("SizeOf Operand:");
  DUMP_TYPE(asmjit::Operand);
  DUMP_TYPE(asmjit::Reg);
  DUMP_TYPE(asmjit::Var);
  DUMP_TYPE(asmjit::BaseMem);
  DUMP_TYPE(asmjit::Imm);
  DUMP_TYPE(asmjit::Label);
  INFO("");

#if !defined(ASMJIT_DISABLE_COMPILER)
  INFO("SizeOf Compiler:");
  DUMP_TYPE(asmjit::Compiler);
  DUMP_TYPE(asmjit::HLNode);
  DUMP_TYPE(asmjit::HLInst);
  DUMP_TYPE(asmjit::HLJump);
  DUMP_TYPE(asmjit::HLData);
  DUMP_TYPE(asmjit::HLAlign);
  DUMP_TYPE(asmjit::HLLabel);
  DUMP_TYPE(asmjit::HLComment);
  DUMP_TYPE(asmjit::HLSentinel);
  DUMP_TYPE(asmjit::HLFunc);
  DUMP_TYPE(asmjit::HLCall);
  DUMP_TYPE(asmjit::FuncDecl);
  DUMP_TYPE(asmjit::FuncInOut);
  DUMP_TYPE(asmjit::FuncPrototype);
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
