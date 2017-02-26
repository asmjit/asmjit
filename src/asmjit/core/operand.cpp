// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::Operand - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(core_operand) {
  INFO("Checking operand sizes");
  EXPECT(sizeof(Operand) == 16);
  EXPECT(sizeof(Reg)     == 16);
  EXPECT(sizeof(Mem)     == 16);
  EXPECT(sizeof(Imm)     == 16);
  EXPECT(sizeof(Label)   == 16);

  INFO("Checking basic functionality of Operand");
  Operand a, b;
  Operand dummy;

  EXPECT(a.isNone() == true);
  EXPECT(a.isReg() == false);
  EXPECT(a.isMem() == false);
  EXPECT(a.isImm() == false);
  EXPECT(a.isLabel() == false);
  EXPECT(a == b);

  EXPECT(a._p32[2] == 0);
  EXPECT(a._p32[3] == 0);

  INFO("Checking basic functionality of Label");
  Label label;
  EXPECT(label.isValid() == false);
  EXPECT(label.getId() == 0);

  INFO("Checking basic functionality of Reg");
  EXPECT(Reg().isReg() == false);
  EXPECT(Reg().isValid() == false);

  EXPECT(Reg()._p32[2] == 0);
  EXPECT(Reg()._p32[3] == 0);
  EXPECT(dummy.as<Reg>().isValid() == false);

  // Create some register (not specific to any architecture).
  uint32_t rSig = Operand::kOpReg | (1 << Operand::kSignatureRegTypeShift ) |
                                    (2 << Operand::kSignatureRegGroupShift) |
                                    (8 << Operand::kSignatureSizeShift    ) ;
  Reg r1(rSig, 5);

  EXPECT(r1.isValid()      == true);
  EXPECT(r1.isReg()        == true);
  EXPECT(r1.isReg(1)       == true);
  EXPECT(r1.isPhysReg()    == true);
  EXPECT(r1.isVirtReg()    == false);
  EXPECT(r1.getSignature() == rSig);
  EXPECT(r1.getType()      == 1);
  EXPECT(r1.getGroup()     == 2);
  EXPECT(r1.getSize()      == 8);
  EXPECT(r1.getId()        == 5);
  EXPECT(r1.isReg(1, 5)    == true); // RegType and Id.
  EXPECT(r1._p32[2] == 0);
  EXPECT(r1._p32[3] == 0);

  // The same type of register having different id.
  Reg r2(r1, 6);
  EXPECT(r2.isValid()      == true);
  EXPECT(r2.isReg()        == true);
  EXPECT(r2.isReg(1)       == true);
  EXPECT(r2.isPhysReg()    == true);
  EXPECT(r2.isVirtReg()    == false);
  EXPECT(r2.getSignature() == rSig);
  EXPECT(r2.getType()      == r1.getType());
  EXPECT(r2.getGroup()     == r1.getGroup());
  EXPECT(r2.getSize()      == r1.getSize());
  EXPECT(r2.getId()        == 6);
  EXPECT(r2.isReg(1, 6)    == true);

  r1.reset();
  EXPECT(!r1.isValid());
  EXPECT(!r1.isReg());

  INFO("Checking basic functionality of Mem");
  Mem m;
  EXPECT(m.isMem());
  EXPECT(m == Mem());
  EXPECT(m.hasBase() == false);
  EXPECT(m.hasIndex() == false);
  EXPECT(m.hasOffset() == false);
  EXPECT(m.isOffset64Bit() == true);
  EXPECT(m.getOffset() == 0);

  m.setOffset(-1);
  EXPECT(m.getOffsetLo32() == -1);
  EXPECT(m.getOffset() == -1);

  int64_t x = int64_t(ASMJIT_UINT64_C(0xFF00FF0000000001));
  int32_t xHi = int32_t(0xFF00FF00);
  m.setOffset(x);
  EXPECT(m.getOffset() == x);
  EXPECT(m.getOffsetLo32() == 1);
  EXPECT(m.getOffsetHi32() == xHi);

  INFO("Checking basic functionality of Imm");
  EXPECT(Imm(-1).getInt64() == int64_t(-1));
}
#endif

ASMJIT_END_NAMESPACE
