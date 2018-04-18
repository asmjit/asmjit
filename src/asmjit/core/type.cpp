// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

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
  #define VALUE(X) Type::BaseOfTypeId<X>::kTypeId
  { ASMJIT_LOOKUP_TABLE_256(VALUE, 0) },
  #undef VALUE

  #define VALUE(X) Type::SizeOfTypeId<X>::kTypeSize
  { ASMJIT_LOOKUP_TABLE_256(VALUE, 0) }
  #undef VALUE
};

ASMJIT_END_NAMESPACE
