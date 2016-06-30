// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS
#define ASMJIT_EXPORTS_X86_REGS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies]
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

#define REG(type, index, size) {{{ \
  Operand::kTypeReg, size, { ((type) << 8) + index }, kInvalidValue, {{ kInvalidVar, 0 }} \
}}}

#define REG_LIST_04(type, start, size) \
  REG(type, start + 0, size), \
  REG(type, start + 1, size), \
  REG(type, start + 2, size), \
  REG(type, start + 3, size)

#define REG_LIST_08(type, start, size) \
  REG_LIST_04(type, start + 0, size), \
  REG_LIST_04(type, start + 4, size)

#define REG_LIST_16(type, start, size) \
  REG_LIST_08(type, start + 0, size), \
  REG_LIST_08(type, start + 8, size)

#define REG_LIST_32(type, start, size) \
  REG_LIST_16(type, start + 0, size), \
  REG_LIST_16(type, start + 16, size)

const X86RegData x86RegData = {
  { REG_LIST_16(kX86RegTypeGpd  , 0,  4) },
  { REG_LIST_16(kX86RegTypeGpq  , 0,  8) },
  { REG_LIST_16(kX86RegTypeGpbLo, 0,  1) },
  { REG_LIST_04(kX86RegTypeGpbHi, 0,  1) },
  { REG_LIST_16(kX86RegTypeGpw  , 0,  2) },
  { REG_LIST_32(kX86RegTypeXmm  , 0, 16) },
  { REG_LIST_32(kX86RegTypeYmm  , 0, 32) },
  { REG_LIST_32(kX86RegTypeZmm  , 0, 64) },
  { REG_LIST_08(kX86RegTypeK    , 0,  8) },
  { REG_LIST_08(kX86RegTypeFp   , 0, 10) },
  { REG_LIST_08(kX86RegTypeMm   , 0,  8) },

  {
    REG(kX86RegTypeSeg, 0, 2), // Default.
    REG(kX86RegTypeSeg, 1, 2), // ES.
    REG(kX86RegTypeSeg, 2, 2), // CS.
    REG(kX86RegTypeSeg, 3, 2), // SS.
    REG(kX86RegTypeSeg, 4, 2), // DS.
    REG(kX86RegTypeSeg, 5, 2), // FS.
    REG(kX86RegTypeSeg, 6, 2)  // GS.
  },

  REG(kInvalidReg, kInvalidReg, 0), // NoGp.
  REG(kX86RegTypeRip, 0, 0),        // RIP.
};

#undef REG_LIST_32
#undef REG_LIST_16
#undef REG_LIST_08
#undef REG_LIST_04
#undef REG

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
