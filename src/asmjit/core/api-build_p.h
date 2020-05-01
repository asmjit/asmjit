// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_API_BUILD_P_H
#define _ASMJIT_CORE_API_BUILD_P_H

#define ASMJIT_EXPORTS

// Dependencies only required for asmjit build, but never exposed through public headers.
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#include "./api-config.h"

// Make sure '#ifdef'ed unit tests are properly highlighted in IDE.
#if !defined(ASMJIT_TEST) && defined(__INTELLISENSE__)
  #define ASMJIT_TEST
#endif

// Include a unit testing package if this is a `asmjit_test_unit` build.
#if defined(ASMJIT_TEST)
  #include "../../../test/broken.h"
#endif

#endif // _ASMJIT_CORE_API_BUILD_P_H
