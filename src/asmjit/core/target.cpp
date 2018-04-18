// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/target.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::Target - Construction / Destruction]
// ============================================================================

Target::Target() noexcept
  : _targetType(kTargetNone),
    _codeInfo() {}
Target::~Target() noexcept {}

ASMJIT_END_NAMESPACE
