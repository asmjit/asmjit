// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

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
