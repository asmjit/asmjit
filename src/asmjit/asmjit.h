// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ASMJIT_H
#define _ASMJIT_ASMJIT_H

// ============================================================================
// [asmjit_mainpage]
// ============================================================================

//! \mainpage
//!
//! AsmJit - Complete x86/x64 JIT and Remote Assembler for C++.
//!
//! Introduction provided by the project page at https://github.com/asmjit/asmjit.

//! \defgroup asmjit_core AsmJit Base API (architecture independent)
//!
//! \brief Backend Neutral API.

//! \defgroup asmjit_x86 AsmJit X86/X64 API
//!
//! \brief X86/X64 Backend API.

//! \defgroup asmjit_arm AsmJit ARM32/ARM64 API
//!
//! \brief ARM32/ARM64 Backend API.

#include "./core.h"

#ifdef ASMJIT_BUILD_X86
  #include "./x86.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "./arm.h"
#endif

// [Guard]
#endif // _ASMJIT_ASMJIT_H
