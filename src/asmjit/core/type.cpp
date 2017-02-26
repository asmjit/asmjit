// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/misc_p.h"
#include "../core/type.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::Type]
// ============================================================================

const Type::TypeData Type::_typeData = {
  { ASMJIT_TABLE_T_256(Type::BaseOfTypeId, kTypeId  , 0) },
  { ASMJIT_TABLE_T_256(Type::SizeOfTypeId, kTypeSize, 0) }
};

ASMJIT_END_NAMESPACE
