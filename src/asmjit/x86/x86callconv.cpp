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
#include "../x86/x86callconv_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::X86CallConvInternal - Init]
// ============================================================================

ASMJIT_FAVOR_SIZE Error X86CallConvInternal::init(CallConv& cc, uint32_t ccId) noexcept {
  constexpr uint32_t kGroupGp  = X86Reg::kGroupGp;
  constexpr uint32_t kGroupVec = X86Reg::kGroupVec;
  constexpr uint32_t kGroupMm  = X86Reg::kGroupMm;
  constexpr uint32_t kGroupK   = X86Reg::kGroupK;

  constexpr uint32_t kZax = X86Gp::kIdAx;
  constexpr uint32_t kZbx = X86Gp::kIdBx;
  constexpr uint32_t kZcx = X86Gp::kIdCx;
  constexpr uint32_t kZdx = X86Gp::kIdDx;
  constexpr uint32_t kZsp = X86Gp::kIdSp;
  constexpr uint32_t kZbp = X86Gp::kIdBp;
  constexpr uint32_t kZsi = X86Gp::kIdSi;
  constexpr uint32_t kZdi = X86Gp::kIdDi;

  switch (ccId) {
    case CallConv::kIdX86StdCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      goto X86CallConv;

    case CallConv::kIdX86MsThisCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx);
      goto X86CallConv;

    case CallConv::kIdX86MsFastCall:
    case CallConv::kIdX86GccFastCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm1:
      cc.setPassedOrder(kGroupGp, kZax);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm2:
      cc.setPassedOrder(kGroupGp, kZax, kZdx);
      goto X86CallConv;

    case CallConv::kIdX86GccRegParm3:
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx);
      goto X86CallConv;

    case CallConv::kIdX86CDecl:
X86CallConv:
      cc.setNaturalStackAlignment(4);
      cc.setArchType(ArchInfo::kTypeX86);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, kZsi, kZdi));
      break;

    case CallConv::kIdX86Win64:
      cc.setArchType(ArchInfo::kTypeX64);
      cc.setStrategy(CallConv::kStrategyWin64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec | CallConv::kFlagIndirectVecArgs);
      cc.setNaturalStackAlignment(16);
      cc.setSpillZoneSize(32);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, kZsi, kZdi, 12, 13, 14, 15));
      cc.setPreservedRegs(kGroupVec, IntUtils::mask(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86SysV64:
      cc.setArchType(ArchInfo::kTypeX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setRedZoneSize(128);
      cc.setPassedOrder(kGroupGp, kZdi, kZsi, kZdx, kZcx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPreservedRegs(kGroupGp, IntUtils::mask(kZbx, kZsp, kZbp, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86LightCall2:
    case CallConv::kIdX86LightCall3:
    case CallConv::kIdX86LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX86LightCall2) + 2;

      cc.setArchType(ArchInfo::kTypeX86);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp , IntUtils::lsbMask<uint32_t>(8));
      cc.setPreservedRegs(kGroupVec, IntUtils::lsbMask<uint32_t>(8) & ~IntUtils::lsbMask<uint32_t>(n));
      cc.setPreservedRegs(kGroupMm , IntUtils::lsbMask<uint32_t>(8));
      cc.setPreservedRegs(kGroupK  , IntUtils::lsbMask<uint32_t>(8));
      break;
    }

    case CallConv::kIdX64LightCall2:
    case CallConv::kIdX64LightCall3:
    case CallConv::kIdX64LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX64LightCall2) + 2;

      cc.setArchType(ArchInfo::kTypeX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp , IntUtils::lsbMask<uint32_t>(16));
      cc.setPreservedRegs(kGroupVec,~IntUtils::lsbMask<uint32_t>(n));
      cc.setPreservedRegs(kGroupMm , IntUtils::lsbMask<uint32_t>(8));
      cc.setPreservedRegs(kGroupK  , IntUtils::lsbMask<uint32_t>(8));
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }

  cc.setId(ccId);
  return kErrorOk;
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86
