// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_AARCH32)
#include <asmjit/a32.h>

#include <limits>
#include <stdio.h>
#include <string.h>

#include <asmjit-testing/bench/asmjit_bench_codegen.h>

using namespace asmjit;

void benchmark_aarch32_emitters(uint32_t num_iterations) {
  Support::maybe_unused(num_iterations);
}

#endif // !ASMJIT_NO_AARCH32
