// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "../core/api-build_p.h"
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
