// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

#include "../core/misc_p.h"
#include "../arm/armoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

const ArmOpData armOpData = {
  {
    // RegInfo[]
    # define VALUE(X) { RegTraits<X>::kSignature }
    { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) },
    #undef VALUE

    // RegCount[]
    # define VALUE(X) RegTraits<X>::kCount
    { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) },
    #undef VALUE

    // RegTypeToTypeId[]
    #define VALUE(X) RegTraits<X>::kTypeId
    { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) }
    #undef VALUE
  }
};
// ============================================================================
// [asmjit::ArmOperand - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(asmjit_arm_operand) {
  Label L;

  INFO("Checking if arm::reg(...) matches built-in IDs");
  EXPECT(w(5) == w5);
  EXPECT(x(5) == x5);

  INFO("Checking GP register properties");
  EXPECT(Gp().isReg() == true);
  EXPECT(w0.isReg() == true);
  EXPECT(x0.isReg() == true);
  EXPECT(w0.id() == 0);
  EXPECT(w31.id() == 31);
  EXPECT(x0.id() == 0);
  EXPECT(x31.id() == 31);
  EXPECT(w0.size() == 4);
  EXPECT(x0.size() == 8);
  EXPECT(w0.type() == Reg::kTypeGpw);
  EXPECT(x0.type() == Reg::kTypeGpx);
  EXPECT(w0.group() == Reg::kGroupGp);
  EXPECT(x0.group() == Reg::kGroupGp);
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_ARM
