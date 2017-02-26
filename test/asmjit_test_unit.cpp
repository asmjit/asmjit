// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies]
#include "./asmjit.h"

using namespace asmjit;

// ============================================================================
// [DumpCpu]
// ============================================================================

struct DumpCpuFeature {
  uint32_t feature;
  const char* name;
};

static void dumpCpuFeatures(const CpuInfo& cpu, const DumpCpuFeature* data, size_t count) {
  for (size_t i = 0; i < count; i++)
    if (cpu.hasFeature(data[i].feature))
      INFO("  %s", data[i].name);
}

static void dumpCpu(void) {
  const CpuInfo& cpu = CpuInfo::getHost();

  INFO("Host CPU:");
  INFO("  Vendor string           : %s", cpu.getVendorString());
  INFO("  Brand string            : %s", cpu.getBrandString());
  INFO("  Model ID                : %u", cpu.getModelId());
  INFO("  Family ID               : %u", cpu.getFamilyId());
  INFO("  Stepping                : %u", cpu.getStepping());
  INFO("  HW-Threads Count        : %u", cpu.getHwThreadsCount());
  INFO("");

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

  #if ASMJIT_ARCH_X86
  static const DumpCpuFeature x86FeaturesList[] = {
    { CpuInfo::kX86FeatureNX              , "NX"               },
    { CpuInfo::kX86FeatureMT              , "MT"               },
    { CpuInfo::kX86FeatureCMOV            , "CMOV"             },
    { CpuInfo::kX86FeatureCMPXCHG8B       , "CMPXCHG8B"        },
    { CpuInfo::kX86FeatureCMPXCHG16B      , "CMPXCHG16B"       },
    { CpuInfo::kX86FeatureMSR             , "MSR"              },
    { CpuInfo::kX86FeatureRDTSC           , "RDTSC"            },
    { CpuInfo::kX86FeatureRDTSCP          , "RDTSCP"           },
    { CpuInfo::kX86FeatureCLFLUSH         , "CLFLUSH"          },
    { CpuInfo::kX86FeatureCLFLUSHOPT      , "CLFLUSHOPT"       },
    { CpuInfo::kX86FeatureCLWB            , "CLWB"             },
    { CpuInfo::kX86FeatureCLZERO          , "CLZERO"           },
    { CpuInfo::kX86FeaturePCOMMIT         , "PCOMMIT"          },
    { CpuInfo::kX86FeaturePREFETCHW       , "PREFETCHW"        },
    { CpuInfo::kX86FeaturePREFETCHWT1     , "PREFETCHWT1"      },
    { CpuInfo::kX86FeatureLAHFSAHF        , "LAHF/SAHF"        },
    { CpuInfo::kX86FeatureFXSR            , "FXSR"             },
    { CpuInfo::kX86FeatureFXSROPT         , "FXSROPT"          },
    { CpuInfo::kX86FeatureMMX             , "MMX"              },
    { CpuInfo::kX86FeatureMMX2            , "MMX2"             },
    { CpuInfo::kX86Feature3DNOW           , "3DNOW"            },
    { CpuInfo::kX86Feature3DNOW2          , "3DNOW2"           },
    { CpuInfo::kX86FeatureSSE             , "SSE"              },
    { CpuInfo::kX86FeatureSSE2            , "SSE2"             },
    { CpuInfo::kX86FeatureSSE3            , "SSE3"             },
    { CpuInfo::kX86FeatureSSSE3           , "SSSE3"            },
    { CpuInfo::kX86FeatureSSE4A           , "SSE4A"            },
    { CpuInfo::kX86FeatureSSE4_1          , "SSE4.1"           },
    { CpuInfo::kX86FeatureSSE4_2          , "SSE4.2"           },
    { CpuInfo::kX86FeatureMSSE            , "MSSE"             },
    { CpuInfo::kX86FeatureMONITOR         , "MONITOR"          },
    { CpuInfo::kX86FeatureMONITORX        , "MONITORX"         },
    { CpuInfo::kX86FeatureMOVBE           , "MOVBE"            },
    { CpuInfo::kX86FeaturePOPCNT          , "POPCNT"           },
    { CpuInfo::kX86FeatureLZCNT           , "LZCNT"            },
    { CpuInfo::kX86FeatureAESNI           , "AESNI"            },
    { CpuInfo::kX86FeaturePCLMULQDQ       , "PCLMULQDQ"        },
    { CpuInfo::kX86FeatureRDRAND          , "RDRAND"           },
    { CpuInfo::kX86FeatureRDSEED          , "RDSEED"           },
    { CpuInfo::kX86FeatureSMAP            , "SMAP"             },
    { CpuInfo::kX86FeatureSMEP            , "SMEP"             },
    { CpuInfo::kX86FeatureSHA             , "SHA"              },
    { CpuInfo::kX86FeatureXSAVE           , "XSAVE"            },
    { CpuInfo::kX86FeatureXSAVEC          , "XSAVEC"           },
    { CpuInfo::kX86FeatureXSAVES          , "XSAVES"           },
    { CpuInfo::kX86FeatureXSAVEOPT        , "XSAVEOPT"         },
    { CpuInfo::kX86FeatureOSXSAVE         , "OSXSAVE"          },
    { CpuInfo::kX86FeatureAVX             , "AVX"              },
    { CpuInfo::kX86FeatureAVX2            , "AVX2"             },
    { CpuInfo::kX86FeatureF16C            , "F16C"             },
    { CpuInfo::kX86FeatureFMA             , "FMA"              },
    { CpuInfo::kX86FeatureFMA4            , "FMA4"             },
    { CpuInfo::kX86FeatureXOP             , "XOP"              },
    { CpuInfo::kX86FeatureBMI             , "BMI"              },
    { CpuInfo::kX86FeatureBMI2            , "BMI2"             },
    { CpuInfo::kX86FeatureADX             , "ADX"              },
    { CpuInfo::kX86FeatureTBM             , "TBM"              },
    { CpuInfo::kX86FeatureMPX             , "MPX"              },
    { CpuInfo::kX86FeatureHLE             , "HLE"              },
    { CpuInfo::kX86FeatureRTM             , "RTM"              },
    { CpuInfo::kX86FeatureTSX             , "TSX"              },
    { CpuInfo::kX86FeatureERMS            , "ERMS"             },
    { CpuInfo::kX86FeatureFSGSBASE        , "FSGSBASE"         },
    { CpuInfo::kX86FeatureAVX512_F        , "AVX512-F"         },
    { CpuInfo::kX86FeatureAVX512_CDI      , "AVX512-CDI"       },
    { CpuInfo::kX86FeatureAVX512_PFI      , "AVX512-PFI"       },
    { CpuInfo::kX86FeatureAVX512_ERI      , "AVX512-ERI"       },
    { CpuInfo::kX86FeatureAVX512_DQ       , "AVX512-DQ"        },
    { CpuInfo::kX86FeatureAVX512_BW       , "AVX512-BW"        },
    { CpuInfo::kX86FeatureAVX512_VL       , "AVX512-VL"        },
    { CpuInfo::kX86FeatureAVX512_IFMA     , "AVX512-IFMA"      },
    { CpuInfo::kX86FeatureAVX512_VBMI     , "AVX512-VBMI"      },
    { CpuInfo::kX86FeatureAVX512_VPOPCNTDQ, "AVX512-VPOPCNTDQ" },
    { CpuInfo::kX86FeatureAVX512_4FMAPS   , "AVX512-4FMAPS"    },
    { CpuInfo::kX86FeatureAVX512_4VNNIW   , "AVX512-4VNNIW"    }
  };

  INFO("X86 Specific:");
  INFO("  Processor Type          : %u", cpu.getX86ProcessorType());
  INFO("  Brand Index             : %u", cpu.getX86BrandIndex());
  INFO("  CL Flush Cache Line     : %u", cpu.getX86FlushCacheLineSize());
  INFO("  Max logical Processors  : %u", cpu.getX86MaxLogicalProcessors());
  INFO("");

  INFO("X86 Features:");
  dumpCpuFeatures(cpu, x86FeaturesList, ASMJIT_ARRAY_SIZE(x86FeaturesList));
  INFO("");
  #endif

  // --------------------------------------------------------------------------
  // [ARM]
  // --------------------------------------------------------------------------

  #if ASMJIT_ARCH_ARM
  static const DumpCpuFeature armFeaturesList[] = {
    { CpuInfo::kArmFeatureV6              , "ARMv6"            },
    { CpuInfo::kArmFeatureV7              , "ARMv7"            },
    { CpuInfo::kArmFeatureV8              , "ARMv8"            },
    { CpuInfo::kArmFeatureTHUMB           , "THUMB"            },
    { CpuInfo::kArmFeatureTHUMB2          , "THUMBv2"          },
    { CpuInfo::kArmFeatureVFP2            , "VFPv2"            },
    { CpuInfo::kArmFeatureVFP3            , "VFPv3"            },
    { CpuInfo::kArmFeatureVFP4            , "VFPv4"            },
    { CpuInfo::kArmFeatureVFP_D32         , "VFP D32"          },
    { CpuInfo::kArmFeatureNEON            , "NEON"             },
    { CpuInfo::kArmFeatureDSP             , "DSP"              },
    { CpuInfo::kArmFeatureIDIV            , "IDIV"             },
    { CpuInfo::kArmFeatureAES             , "AES"              },
    { CpuInfo::kArmFeatureCRC32           , "CRC32"            },
    { CpuInfo::kArmFeatureSHA1            , "SHA1"             },
    { CpuInfo::kArmFeatureSHA256          , "SHA256"           },
    { CpuInfo::kArmFeatureATOMIC64        , "ATOMIC64"         }
  };

  INFO("ARM Features:");
  dumpCpuFeatures(cpu, armFeaturesList, ASMJIT_ARRAY_SIZE(armFeaturesList));
  INFO("");
  #endif
}

// ============================================================================
// [DumpSizeOf]
// ============================================================================

static void dumpSizeOf(void) {
  #define DUMP_TYPE(...) \
    INFO("  %-26s: %u", #__VA_ARGS__, uint32_t(sizeof(__VA_ARGS__)))

  INFO("Size of built-ins:");
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

  INFO("Size of Base:");
    DUMP_TYPE(Assembler);
    DUMP_TYPE(CodeBuffer);
    DUMP_TYPE(CodeEmitter);
    DUMP_TYPE(CodeHolder);
    DUMP_TYPE(ConstPool);
    DUMP_TYPE(LabelEntry);
    DUMP_TYPE(RelocEntry);
    DUMP_TYPE(SectionEntry);
    DUMP_TYPE(StringBuilder);
    DUMP_TYPE(Target);
    DUMP_TYPE(Zone);
    DUMP_TYPE(ZoneAllocator);
    DUMP_TYPE(ZoneHashNode);
    DUMP_TYPE(ZoneHash<ZoneHashNode>);
    DUMP_TYPE(ZoneList<void*>);
    DUMP_TYPE(ZoneVector<void*>);
    DUMP_TYPE(ZoneBitVector);
  INFO("");

  INFO("Size of Operand:");
    DUMP_TYPE(Operand);
    DUMP_TYPE(Reg);
    DUMP_TYPE(Mem);
    DUMP_TYPE(Imm);
    DUMP_TYPE(Label);
  INFO("");

  INFO("Size of Func:");
    DUMP_TYPE(CallConv);
    DUMP_TYPE(FuncFrame);
    DUMP_TYPE(FuncValue);
    DUMP_TYPE(FuncDetail);
    DUMP_TYPE(FuncSignature);
    DUMP_TYPE(FuncArgsAssignment);
  INFO("");

  #ifndef ASMJIT_DISABLE_BUILDER
  INFO("Size of CodeBuilder:");
    DUMP_TYPE(CodeBuilder);
    DUMP_TYPE(CBNode);
    DUMP_TYPE(CBInst);
    DUMP_TYPE(CBInstEx);
    DUMP_TYPE(CBData);
    DUMP_TYPE(CBAlign);
    DUMP_TYPE(CBLabel);
    DUMP_TYPE(CBComment);
    DUMP_TYPE(CBSentinel);
  INFO("");
  #endif

  #ifndef ASMJIT_DISABLE_COMPILER
  INFO("Size of CodeCompiler:");
    DUMP_TYPE(CodeCompiler);
    DUMP_TYPE(CCFunc);
    DUMP_TYPE(CCFuncRet);
    DUMP_TYPE(CCFuncCall);
  INFO("");
  #endif

  #ifdef ASMJIT_BUILD_X86
  INFO("Size of X86-Backend:");
    DUMP_TYPE(X86Assembler);
    #ifndef ASMJIT_DISABLE_BUILDER
    DUMP_TYPE(X86Builder);
    #endif
    #ifndef ASMJIT_DISABLE_COMPILER
    DUMP_TYPE(X86Compiler);
    #endif
    DUMP_TYPE(X86Inst);
    DUMP_TYPE(X86Inst::CommonData);
    DUMP_TYPE(X86Inst::OperationData);
    DUMP_TYPE(X86Inst::SseToAvxData);
    DUMP_TYPE(X86Inst::ISignature);
    DUMP_TYPE(X86Inst::OSignature);
  INFO("");
  #endif

  #undef DUMP_TYPE
}

// ============================================================================
// [Main]
// ============================================================================

static void onBeforeRun(void) {
  dumpCpu();
  dumpSizeOf();
}

int main(int argc, const char* argv[]) {
  #if defined(ASMJIT_BUILD_DEBUG)
  const char buildType[] = "DEBUG";
  #else
  const char buildType[] = "RELEASE";
  #endif

  INFO("AsmJit Unit-Test (v%u.%u.%u [%s])\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    buildType
  );
  return BrokenAPI::run(argc, argv, onBeforeRun);
}
