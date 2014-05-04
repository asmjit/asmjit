// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_H
#define _ASMJIT_X86_H

// ============================================================================
// [asmjit_x86x64]
// ============================================================================

//! @defgroup asmjit_x86x64 X86/X64
//!
//! @brief X86/X64 API

// ============================================================================
// [asmjit_x86x64_codegen]
// ============================================================================

//! @defgroup asmjit_x86x64_codegen Code Generation (X86/X64)
//! @ingroup asmjit_x86x64
//!
//! @brief Low-level and high-level code generation.

// ============================================================================
// [asmjit_x86x64_cpu_info]
// ============================================================================

//! @defgroup asmjit_x86x64_cpu_info CPU Information (X86/X64)
//! @ingroup asmjit_x86x64
//!
//! @brief CPU information specific to X86/X64 architecture.
//!
//! The CPUID instruction can be used to get an exhaustive information related
//! to the host X86/X64 processor. AsmJit contains utilities that can get the
//! most important information related to the features supported by the CPU
//! and the host operating system, in addition to host processor name and number
//! of cores. Class `CpuInfo` extends `BaseCpuInfo` and provides functionality
//! specific to X86 and X64.
//!
//! By default AsmJit queries the CPU information after the library is loaded
//! and the queried information is reused by all instances of `JitRuntime`.
//! The global instance of `CpuInfo` can't be changed, because it will affect
//! the code generation of all `Runtime`s. If there is a need to have a 
//! specific CPU information which contains modified features or processor
//! vendor it's possible by creating a new instance of `CpuInfo` and setting
//! up its members. `CpuUtil::detect` can be used to detect CPU features into
//! an existing `CpuInfo` instance - it may become handly if only one property
//! has to be turned on/off.
//!
//! If the high-level interface `CpuInfo` offers is not enough there is also
//! `CpuUtil::callCpuId` helper that can be used to call CPUID instruction with
//! a given parameters and to consume the output.
//!
//! Cpu detection is important when generating a JIT code that may or may not
//! use certain CPU features. For example there used to be a SSE/SSE2 detection
//! in the past and today there is often AVX/AVX2 detection.
//!
//! The example below shows how to detect SSE2:
//! 
//! ~~~
//! using namespace asmjit;
//! using namespace asmjit::host;
//!
//! // Get `CpuInfo` global instance.
//! const CpuInfo* cpuInfo = CpuInfo::getHost();
//!
//! if (cpuInfo->hasFeature(kCpuFeatureSse2)) {
//!   // Processor has SSE2.
//! }
//! else if (cpuInfo->hasFeature(kCpuFeatureMmx)) {
//!   // Processor doesn't have SSE2, but has MMX.
//! }
//! else {
//!   // An archaic processor, it's a wonder AsmJit works here!
//! }
//! ~~~
//! 
//! The next example shows how to call CPUID directly:
//!
//! ~~~
//! using namespace asmjit;
//!
//! // The result of CPUID call.
//! CpuId out;
//!
//! // Call CPUID, first two arguments are passed in EAX/ECX.
//! CpuUtil::callCpuId(0, 0, &out);
//!
//! // If EAX argument is 0, EBX, ECX and EDX registers are filled with a cpu vendor.
//! char cpuVendor[13];
//! memcpy(cpuVendor, &out.ebx, 4);
//! memcpy(cpuVendor + 4, &out.edx, 4);
//! memcpy(cpuVendor + 8, &out.ecx, 4);
//! vendor[12] = '\0';
//!
//! // Print a CPU vendor retrieved from CPUID.
//! ::printf("%s", cpuVendor);
//! ~~~
//!
//! @sa @ref asmjit_base_cpu_info

// ============================================================================
// [asmjit_x86x64_constants]
// ============================================================================

//! @defgroup asmjit_x86x64_constants Constants (X86/X64)
//! @ingroup asmjit_x86x64
//!
//! @brief Constants and definitions specific to X86/X64 architecture.

// ============================================================================
// [Dependencies - AsmJit]
// ============================================================================

#include "base.h"

#include "x86/x86assembler.h"
#include "x86/x86compiler.h"
#include "x86/x86cpuinfo.h"
#include "x86/x86defs.h"
#include "x86/x86func.h"

// [Guard]
#endif // _ASMJIT_X86_H
