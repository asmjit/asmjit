// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && ASMJIT_ARCH_X86

#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::Inst]
// ============================================================================

//! Definitions for Inst static members.
constexpr uint16_t Inst::jccTable[];
constexpr uint16_t Inst::setccTable[];
constexpr uint16_t Inst::cmovccTable[];

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86 && ASMJIT_ARCH_X86
