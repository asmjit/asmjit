// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <asmjit/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_assembler.h"
#include "cmdline.h"

using namespace asmjit;

#if defined(ASMJIT_BUILD_ARM)
bool testAArch64Assembler(const TestSettings& settings) noexcept;
#endif

int main(int argc, char* argv[]) {
  CmdLine cmdLine(argc, argv);

  TestSettings settings {};
  settings.quiet = cmdLine.hasArg("--quiet");

  printf("AsmJit Assembler Test-Suite v%u.%u.%u:\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF));
  printf("  [%s] Quiet (use --quiet option to only show errors)\n", settings.quiet ? "x" : " ");
  printf("\n");

  bool failed = false;

#if defined(ASMJIT_BUILD_ARM)
  if (!testAArch64Assembler(settings))
    failed = true;
#endif

  return failed ? 1 : 0;
}
