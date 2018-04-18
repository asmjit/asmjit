// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ASMJIT_H
#define _ASMJIT_ASMJIT_H

// ============================================================================
// [asmjit_mainpage]
// ============================================================================

//! \mainpage
//!
//! AsmJit Reference Manual
//!
//! Introduction provided by the project page at https://github.com/asmjit/asmjit.

// ============================================================================
// [asmjit_core]
// ============================================================================

//! \defgroup asmjit_core asmjit/core
//! \brief AsmJit Core.

//! \defgroup asmjit_core_api Core API
//! \ingroup asmjit_core

//! \defgroup asmjit_core_func Function API
//! \ingroup asmjit_core

//! \defgroup asmjit_core_jit JIT API
//! \ingroup asmjit_core

//! \defgroup asmjit_core_support Support API
//! \ingroup asmjit_core

//! \defgroup asmjit_core_ra RA Internals
//! \ingroup asmjit_core

// ============================================================================
// [asmjit_x86]
// ============================================================================

//! \defgroup asmjit_x86 asmjit/x86
//! \brief AsmJit X86 backend.

//! \defgroup asmjit_x86_api X86 API
//! \ingroup asmjit_x86

//! \defgroup asmjit_x86_db X86 Database
//! \ingroup asmjit_x86

//! \defgroup asmjit_x86_ra X86 RA
//! \ingroup asmjit_x86

// ============================================================================
// [asmjit_arm]
// ============================================================================

//! \defgroup asmjit_arm asmjit/arm
//! \brief AsmJit ARM backend.

//! \defgroup asmjit_arm_api ARM API
//! \ingroup asmjit_arm

//! \defgroup asmjit_arm_db ARM Database
//! \ingroup asmjit_arm

// ============================================================================
// [Dependencies]
// ============================================================================

#include "./core.h"

#ifdef ASMJIT_BUILD_X86
  #include "./x86.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "./arm.h"
#endif

// [Guard]
#endif // _ASMJIT_ASMJIT_H
