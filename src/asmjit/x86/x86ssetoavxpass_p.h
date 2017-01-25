// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86SSETOAVXPASS_P_H
#define _ASMJIT_X86_X86SSETOAVXPASS_P_H

#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_BUIILDER)

// [Dependencies]
#include "../base/codebuilder.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

struct X86SseToAvxPassInit {
  static Error add(CodeBuilder* cb) noexcept;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_BUIILDER
#endif // _ASMJIT_X86_X86SSETOAVXPASS_P_H
