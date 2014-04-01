// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CPUTICKS_H
#define _ASMJIT_BASE_CPUTICKS_H

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::CpuTicks]
// ============================================================================

struct CpuTicks {
  static ASMJIT_API uint32_t now();
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CPUTICKS_H
