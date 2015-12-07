// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Operand]
// ============================================================================

// Prevent static initialization.
struct Operand {
  struct BaseOp {
    uint8_t op;
    uint8_t size;
    uint8_t reserved_2_1;
    uint8_t reserved_3_1;

    uint32_t id;

    uint32_t reserved_8_4;
    uint32_t reserved_12_4;
  };

  // Kept in union to prevent LTO warnings.
  union {
    BaseOp _base;

    // Required to properly align this _fake_ `Operand`, not used.
    uint64_t _data[2];
  };
};

ASMJIT_VARAPI const Operand noOperand;
const Operand noOperand = {{ 0, 0, 0, 0, kInvalidValue, 0, 0 }};

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
