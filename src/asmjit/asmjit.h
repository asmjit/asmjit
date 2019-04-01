// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ASMJIT_H
#define _ASMJIT_ASMJIT_H

//! \mainpage
//!
//! AsmJit Reference Manual
//!
//! Introduction provided by the project page at https://github.com/asmjit/asmjit.
//!
//! \ref asmjit_core

//! \namespace asmjit
//!
//! Root namespace used by AsmJit.

#include "./core.h"

#ifdef ASMJIT_BUILD_X86
  #include "./x86.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "./arm.h"
#endif

#endif // _ASMJIT_ASMJIT_H
