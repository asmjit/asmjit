// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ASSERT_H
#define _ASMJIT_BASE_ASSERT_H

// [Dependencies - AsmJit]
#include "../build.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::Assert]
// ============================================================================

//! @brief Called in debug build on assertion failure.
//!
//! @param exp Expression that failed.
//! @param file Source file name where it happened.
//! @param line Line in the source file.
//!
//! If you have problems with assertions put a breakpoint at assertionFailed()
//! function (asmjit/base/assert.cpp) to see what happened.
ASMJIT_API void assertionFailed(const char* exp, const char* file, int line);

// ============================================================================
// [ASMJIT_ASSERT]
// ============================================================================

#if defined(ASMJIT_DEBUG)
#define ASMJIT_ASSERT(_Exp_) \
  do { \
    if (!(_Exp_)) ::asmjit::assertionFailed(#_Exp_, __FILE__, __LINE__); \
  } while (0)
#else
#define ASMJIT_ASSERT(_Exp_) ASMJIT_NOP()
#endif // DEBUG

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ASSERT_H
