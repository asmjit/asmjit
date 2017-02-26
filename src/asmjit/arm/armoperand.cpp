// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

// [Dependencies]
#include "../core/misc_p.h"
#include "../arm/armoperand.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

const ArmOpData armOpData = {
  {
    {
      #define ASMJIT_ARM_REG_SIGNATURE(TYPE) { ArmRegTraits<TYPE>::kSignature }
      ASMJIT_TABLE_32(ASMJIT_ARM_REG_SIGNATURE, 0)
      #undef ASMJIT_ARM_REG_SIGNATURE
    },

    // RegCount[]
    { ASMJIT_TABLE_T_32(ArmRegTraits, kCount, 0) },

    // RegTypeToTypeId[]
    { ASMJIT_TABLE_T_32(ArmRegTraits, kTypeId, 0) }
  }
};
// ============================================================================
// [asmjit::ArmOperand - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(arm_operand) {
  Label L;

  INFO("Checking if arm::reg(...) matches built-in IDs");
  EXPECT(arm::w(5) == arm::w5);
  EXPECT(arm::x(5) == arm::x5);

  INFO("Checking GP register properties");
  EXPECT(ArmGp().isReg() == false);
  EXPECT(arm::w0.isReg() == true);
  EXPECT(arm::x0.isReg() == true);
  EXPECT(arm::w0.getId() == 0);
  EXPECT(arm::w31.getId() == 31);
  EXPECT(arm::x0.getId() == 0);
  EXPECT(arm::x31.getId() == 31);
  EXPECT(arm::w0.getSize() == 4);
  EXPECT(arm::x0.getSize() == 8);
  EXPECT(arm::w0.getType() == ArmReg::kRegGpw);
  EXPECT(arm::x0.getType() == ArmReg::kRegGpx);
  EXPECT(arm::w0.getGroup() == ArmReg::kGroupGp);
  EXPECT(arm::x0.getGroup() == ArmReg::kGroupGp);
}
#endif

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_ARM
