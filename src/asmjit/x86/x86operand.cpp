// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_X86

// [Dependencies]
#include "../core/misc_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::X86OpData]
// ============================================================================

const X86OpData x86OpData = {
  {
    {
#define ASMJIT_X86_REG_SIGNATURE(TYPE) { X86RegTraits<TYPE>::kSignature }
      ASMJIT_TABLE_16(ASMJIT_X86_REG_SIGNATURE,  0),
      ASMJIT_TABLE_16(ASMJIT_X86_REG_SIGNATURE, 16)
#undef ASMJIT_X86_REG_SIGNATURE
    },

    // RegCount[]
    { ASMJIT_TABLE_T_32(X86RegTraits, kCount, 0) },

    // RegTypeToTypeId[]
    { ASMJIT_TABLE_T_32(X86RegTraits, kTypeId, 0) }
  }
};

// ============================================================================
// [asmjit::X86Operand - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(x86_operand) {
  Label L(1000); // Label with some ID.

  INFO("Checking basic properties of built-in X86 registers");
  EXPECT(x86::gpb(X86Gp::kIdAx) == x86::al);
  EXPECT(x86::gpb(X86Gp::kIdBx) == x86::bl);
  EXPECT(x86::gpb(X86Gp::kIdCx) == x86::cl);
  EXPECT(x86::gpb(X86Gp::kIdDx) == x86::dl);

  EXPECT(x86::gpb_lo(X86Gp::kIdAx) == x86::al);
  EXPECT(x86::gpb_lo(X86Gp::kIdBx) == x86::bl);
  EXPECT(x86::gpb_lo(X86Gp::kIdCx) == x86::cl);
  EXPECT(x86::gpb_lo(X86Gp::kIdDx) == x86::dl);

  EXPECT(x86::gpb_hi(X86Gp::kIdAx) == x86::ah);
  EXPECT(x86::gpb_hi(X86Gp::kIdBx) == x86::bh);
  EXPECT(x86::gpb_hi(X86Gp::kIdCx) == x86::ch);
  EXPECT(x86::gpb_hi(X86Gp::kIdDx) == x86::dh);

  EXPECT(x86::gpw(X86Gp::kIdAx) == x86::ax);
  EXPECT(x86::gpw(X86Gp::kIdBx) == x86::bx);
  EXPECT(x86::gpw(X86Gp::kIdCx) == x86::cx);
  EXPECT(x86::gpw(X86Gp::kIdDx) == x86::dx);

  EXPECT(x86::gpd(X86Gp::kIdAx) == x86::eax);
  EXPECT(x86::gpd(X86Gp::kIdBx) == x86::ebx);
  EXPECT(x86::gpd(X86Gp::kIdCx) == x86::ecx);
  EXPECT(x86::gpd(X86Gp::kIdDx) == x86::edx);

  EXPECT(x86::gpq(X86Gp::kIdAx) == x86::rax);
  EXPECT(x86::gpq(X86Gp::kIdBx) == x86::rbx);
  EXPECT(x86::gpq(X86Gp::kIdCx) == x86::rcx);
  EXPECT(x86::gpq(X86Gp::kIdDx) == x86::rdx);

  EXPECT(x86::gpb(X86Gp::kIdAx) != x86::dl);
  EXPECT(x86::gpw(X86Gp::kIdBx) != x86::cx);
  EXPECT(x86::gpd(X86Gp::kIdCx) != x86::ebx);
  EXPECT(x86::gpq(X86Gp::kIdDx) != x86::rax);

  INFO("Checking if x86::reg(...) matches built-in IDs");
  EXPECT(x86::fp(5)  == x86::fp5);
  EXPECT(x86::mm(5)  == x86::mm5);
  EXPECT(x86::k(5)   == x86::k5);
  EXPECT(x86::cr(5)  == x86::cr5);
  EXPECT(x86::dr(5)  == x86::dr5);
  EXPECT(x86::xmm(5) == x86::xmm5);
  EXPECT(x86::ymm(5) == x86::ymm5);
  EXPECT(x86::zmm(5) == x86::zmm5);

  INFO("Checking GP register properties");
  EXPECT(X86Gp().isReg() == false);
  EXPECT(x86::eax.isReg() == true);
  EXPECT(x86::eax.getId() == 0);
  EXPECT(x86::eax.getSize() == 4);
  EXPECT(x86::eax.getType() == X86Reg::kRegGpd);
  EXPECT(x86::eax.getGroup() == X86Reg::kGroupGp);

  INFO("Checking FP register properties");
  EXPECT(X86Fp().isReg() == false);
  EXPECT(x86::fp1.isReg() == true);
  EXPECT(x86::fp1.getId() == 1);
  EXPECT(x86::fp1.getSize() == 10);
  EXPECT(x86::fp1.getType() == X86Reg::kRegFp);
  EXPECT(x86::fp1.getGroup() == X86Reg::kGroupFp);

  INFO("Checking MM register properties");
  EXPECT(X86Mm().isReg() == false);
  EXPECT(x86::mm2.isReg() == true);
  EXPECT(x86::mm2.getId() == 2);
  EXPECT(x86::mm2.getSize() == 8);
  EXPECT(x86::mm2.getType() == X86Reg::kRegMm);
  EXPECT(x86::mm2.getGroup() == X86Reg::kGroupMm);

  INFO("Checking K register properties");
  EXPECT(X86KReg().isReg() == false);
  EXPECT(x86::k3.isReg() == true);
  EXPECT(x86::k3.getId() == 3);
  EXPECT(x86::k3.getSize() == 0);
  EXPECT(x86::k3.getType() == X86Reg::kRegK);
  EXPECT(x86::k3.getGroup() == X86Reg::kGroupK);

  INFO("Checking XMM register properties");
  EXPECT(X86Xmm().isReg() == false);
  EXPECT(x86::xmm4.isReg() == true);
  EXPECT(x86::xmm4.getId() == 4);
  EXPECT(x86::xmm4.getSize() == 16);
  EXPECT(x86::xmm4.getType() == X86Reg::kRegXmm);
  EXPECT(x86::xmm4.getGroup() == X86Reg::kGroupVec);
  EXPECT(x86::xmm4.isVec());

  INFO("Checking YMM register properties");
  EXPECT(X86Ymm().isReg() == false);
  EXPECT(x86::ymm5.isReg() == true);
  EXPECT(x86::ymm5.getId() == 5);
  EXPECT(x86::ymm5.getSize() == 32);
  EXPECT(x86::ymm5.getType() == X86Reg::kRegYmm);
  EXPECT(x86::ymm5.getGroup() == X86Reg::kGroupVec);
  EXPECT(x86::ymm5.isVec());

  INFO("Checking ZMM register properties");
  EXPECT(X86Zmm().isReg() == false);
  EXPECT(x86::zmm6.isReg() == true);
  EXPECT(x86::zmm6.getId() == 6);
  EXPECT(x86::zmm6.getSize() == 64);
  EXPECT(x86::zmm6.getType() == X86Reg::kRegZmm);
  EXPECT(x86::zmm6.getGroup() == X86Reg::kGroupVec);
  EXPECT(x86::zmm6.isVec());

  INFO("Checking VEC register properties");
  EXPECT(X86Vec().isReg() == false);
  // Converts a VEC register to a type of the passed register, but keeps the ID.
  EXPECT(x86::xmm4.cloneAs(x86::ymm10) == x86::ymm4);
  EXPECT(x86::xmm4.cloneAs(x86::zmm11) == x86::zmm4);
  EXPECT(x86::ymm5.cloneAs(x86::xmm12) == x86::xmm5);
  EXPECT(x86::ymm5.cloneAs(x86::zmm13) == x86::zmm5);
  EXPECT(x86::zmm6.cloneAs(x86::xmm14) == x86::xmm6);
  EXPECT(x86::zmm6.cloneAs(x86::ymm15) == x86::ymm6);

  INFO("Checking if default constructed regs behave as expected");
  EXPECT(X86Reg().isValid() == false);
  EXPECT(X86Gp().isValid() == false);
  EXPECT(X86Fp().isValid() == false);
  EXPECT(X86Mm().isValid() == false);
  EXPECT(X86Xmm().isValid() == false);
  EXPECT(X86Ymm().isValid() == false);
  EXPECT(X86Zmm().isValid() == false);
  EXPECT(X86KReg().isValid() == false);

  INFO("Checking X86Mem operand");
  X86Mem m;
  EXPECT(m == X86Mem(),
    "Two default constructed X86Mem operands must be equal");

  m = x86::ptr(L);
  EXPECT(m.hasBase() == true);
  EXPECT(m.hasBaseReg() == false);
  EXPECT(m.hasBaseLabel() == true);
  EXPECT(m.hasOffset() == false);
  EXPECT(m.isOffset64Bit() == false);
  EXPECT(m.getOffset() == 0);
  EXPECT(m.getOffsetLo32() == 0);

  m = x86::ptr(0x0123456789ABCDEFU);
  EXPECT(m.hasBase() == false);
  EXPECT(m.hasBaseReg() == false);
  EXPECT(m.hasIndex() == false);
  EXPECT(m.hasIndexReg() == false);
  EXPECT(m.hasOffset() == true);
  EXPECT(m.isOffset64Bit() == true);
  EXPECT(m.getOffset() == int64_t(0x0123456789ABCDEFU));
  EXPECT(m.getOffsetLo32() == int32_t(0x89ABCDEFU));
  m.addOffset(1);
  EXPECT(m.getOffset() == int64_t(0x0123456789ABCDF0U));

  m = x86::ptr(0x0123456789ABCDEFU, x86::rdi, 4);
  EXPECT(m.hasBase() == false);
  EXPECT(m.hasBaseReg() == false);
  EXPECT(m.hasIndex() == true);
  EXPECT(m.hasIndexReg() == true);
  EXPECT(m.getIndexType() == x86::rdi.getType());
  EXPECT(m.getIndexId() == x86::rdi.getId());
  EXPECT(m.hasOffset() == true);
  EXPECT(m.isOffset64Bit() == true);
  EXPECT(m.getOffset() == int64_t(0x0123456789ABCDEFU));
  EXPECT(m.getOffsetLo32() == int32_t(0x89ABCDEFU));
  m.resetIndex();
  EXPECT(m.hasIndex() == false);
  EXPECT(m.hasIndexReg() == false);

  m = x86::ptr(x86::rax);
  EXPECT(m.hasBase() == true);
  EXPECT(m.hasBaseReg() == true);
  EXPECT(m.getBaseType() == x86::rax.getType());
  EXPECT(m.getBaseId() == x86::rax.getId());
  EXPECT(m.hasIndex() == false);
  EXPECT(m.hasIndexReg() == false);
  EXPECT(m.getIndexType() == 0);
  EXPECT(m.getIndexId() == 0);
  EXPECT(m.hasOffset() == false);
  EXPECT(m.isOffset64Bit() == false);
  EXPECT(m.getOffset() == 0);
  EXPECT(m.getOffsetLo32() == 0);
  m.setIndex(x86::rsi);
  EXPECT(m.hasIndex() == true);
  EXPECT(m.hasIndexReg() == true);
  EXPECT(m.getIndexType() == x86::rsi.getType());
  EXPECT(m.getIndexId() == x86::rsi.getId());
}
#endif

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86
