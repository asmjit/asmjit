// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../base/intutil.h"
#include "../base/string.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86context_p.h"
#include "../x86/x86cpu.h"
#include "../x86/x86func.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

// ============================================================================
// [Forward Declarations]
// ============================================================================

static Error X86X64Context_translateOperands(X86X64Context* self, Operand* opList, uint32_t opCount);

// ============================================================================
// [asmjit::x86x64::X86X64Context - Construction / Destruction]
// ============================================================================

X86X64Context::X86X64Context(X86X64Compiler* compiler) : BaseContext(compiler) {
  // Setup x86 specific data.
#if defined(ASMJIT_BUILD_X86)
  if (compiler->getArch() == kArchX86) {
    _zsp = x86::esp;
    _zbp = x86::ebp;
    _memSlot._vmem.type = kMemTypeStackIndex;
    _memSlot.setGpdBase(true);
    _baseRegsCount = x86::kRegCountGp;
  }
#endif // ASMJIT_BUILD_X86

  // Setup x64 specific data.
#if defined(ASMJIT_BUILD_X64)
  if (compiler->getArch() == kArchX64) {
    _zsp = x64::rsp;
    _zbp = x64::rbp;
    _memSlot._vmem.type = kMemTypeStackIndex;
    _memSlot.setGpdBase(false);
    _baseRegsCount = x64::kRegCountGp;
  }
#endif // ASMJIT_BUILD_X64

  _state = &_x86State;
  _emitComments = compiler->getLogger() != NULL;

  reset();
}

X86X64Context::~X86X64Context() {}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Reset]
// ============================================================================

void X86X64Context::reset() {
  BaseContext::reset();

  _x86State.reset(0);
  _clobberedRegs.reset();

  _stackFrameCell = NULL;
  _gaRegs[kRegClassGp] = IntUtil::bits(_baseRegsCount) & ~IntUtil::mask(kRegIndexSp);
  _gaRegs[kRegClassFp] = IntUtil::bits(kRegCountFp);
  _gaRegs[kRegClassMm] = IntUtil::bits(kRegCountMm);
  _gaRegs[kRegClassXy] = IntUtil::bits(_baseRegsCount);

  _argBaseReg = kInvalidReg; // Used by patcher.
  _varBaseReg = kInvalidReg; // Used by patcher.

  _argBaseOffset = 0;        // Used by patcher.
  _varBaseOffset = 0;        // Used by patcher.

  _argActualDisp = 0;        // Used by translator.
  _varActualDisp = 0;        // Used by translator.
}

// ============================================================================
// [asmjit::x86x64::X86X64SpecialInst]
// ============================================================================

struct X86X64SpecialInst {
  uint8_t inReg;
  uint8_t outReg;
  uint16_t flags;
};

static const X86X64SpecialInst x86SpecialInstCpuid[] = {
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  },
  { kInvalidReg, kRegIndexBx, kVarAttrOutReg    },
  { kInvalidReg, kRegIndexCx, kVarAttrOutReg    },
  { kInvalidReg, kRegIndexDx, kVarAttrOutReg    }
};

static const X86X64SpecialInst x86SpecialInstCbwCdqeCwde[] = {
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstCdqCwdCqo[] = {
  { kInvalidReg, kRegIndexDx, kVarAttrOutReg    },
  { kRegIndexAx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstCmpxchg[] = {
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  },
  { kInvalidReg, kInvalidReg, kVarAttrInOutReg  },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstCmpxchg8b16b[] = {
  { kRegIndexDx, kRegIndexDx, kVarAttrInOutReg  },
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  },
  { kRegIndexCx, kInvalidReg, kVarAttrInReg     },
  { kRegIndexBx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstDaaDas[] = {
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstDiv[] = {
  { kInvalidReg, kRegIndexDx, kVarAttrInOutReg  },
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstMul[] = {
  { kInvalidReg, kRegIndexDx, kVarAttrOutReg    },
  { kRegIndexAx, kRegIndexAx, kVarAttrInOutReg  },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstMovPtr[] = {
  { kInvalidReg, kRegIndexAx, kVarAttrOutReg    },
  { kRegIndexAx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstLahf[] = {
  { kInvalidReg, kRegIndexAx, kVarAttrOutReg    }
};

static const X86X64SpecialInst x86SpecialInstSahf[] = {
  { kRegIndexAx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstMaskmovqMaskmovdqu[] = {
  { kInvalidReg, kRegIndexDi, kVarAttrInReg     },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstRot[] = {
  { kInvalidReg, kInvalidReg, kVarAttrInOutReg  },
  { kRegIndexCx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstShlrd[] = {
  { kInvalidReg, kInvalidReg, kVarAttrInOutReg  },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     },
  { kRegIndexCx, kInvalidReg, kVarAttrInReg     }
};

static const X86X64SpecialInst x86SpecialInstRdtscRdtscp[] = {
  { kInvalidReg, kRegIndexDx, kVarAttrOutReg    },
  { kInvalidReg, kRegIndexAx, kVarAttrOutReg    },
  { kInvalidReg, kRegIndexCx, kVarAttrOutReg    }
};

static const X86X64SpecialInst x86SpecialInstRepLod[] = {
  { kInvalidReg, kRegIndexAx, kVarAttrOutReg    },
  { kRegIndexSi, kInvalidReg, kVarAttrInReg     },
  { kRegIndexCx, kRegIndexCx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstRepMovCmp[] = {
  { kRegIndexDi, kInvalidReg, kVarAttrInReg     },
  { kRegIndexSi, kInvalidReg, kVarAttrInReg     },
  { kRegIndexCx, kRegIndexCx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstRepSto[] = {
  { kRegIndexDi, kInvalidReg, kVarAttrInReg     },
  { kRegIndexAx, kInvalidReg, kVarAttrInReg     },
  { kRegIndexCx, kRegIndexCx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstRepSca[] = {
  { kRegIndexDi, kInvalidReg, kVarAttrInReg     },
  { kRegIndexAx, kInvalidReg, kVarAttrInReg     },
  { kRegIndexCx, kRegIndexCx, kVarAttrInOutReg  }
};

static const X86X64SpecialInst x86SpecialInstBlend[] = {
  { kInvalidReg, kInvalidReg, kVarAttrOutReg    },
  { kInvalidReg, kInvalidReg, kVarAttrInReg     },
  { 0          , kInvalidReg, kVarAttrInReg     }
};

static ASMJIT_INLINE const X86X64SpecialInst* X86X64SpecialInst_get(uint32_t code, const Operand* opList, uint32_t opCount) {
  switch (code) {
    case kInstCpuid:
      return x86SpecialInstCpuid;

    case kInstCbw:
    case kInstCdqe:
    case kInstCwde:
      return x86SpecialInstCbwCdqeCwde;

    case kInstCdq:
    case kInstCwd:
    case kInstCqo:
      return x86SpecialInstCdqCwdCqo;

    case kInstCmpxchg:
      return x86SpecialInstCmpxchg;

    case kInstCmpxchg8b:
    case kInstCmpxchg16b:
      return x86SpecialInstCmpxchg8b16b;

    case kInstDaa:
    case kInstDas:
      return x86SpecialInstDaaDas;

    case kInstIdiv:
    case kInstDiv:
      return x86SpecialInstDiv;

    case kInstImul:
      if (opCount == 2)
        return NULL;
      if (opCount == 3 && !(opList[0].isVar() && opList[1].isVar() && opList[2].isVarOrMem()))
        return NULL;
      // ... Fall through ...
    case kInstMul:
      return x86SpecialInstMul;

    case kInstMovptr:
      return x86SpecialInstMovPtr;

    case kInstLahf:
      return x86SpecialInstLahf;

    case kInstSahf:
      return x86SpecialInstSahf;

    case kInstMaskmovq:
    case kInstMaskmovdqu:
      return x86SpecialInstMaskmovqMaskmovdqu;

    // Not supported.
    case kInstEnter:
    case kInstLeave:
      return NULL;

    // Not supported.
    case kInstRet:
      return NULL;

    case kInstMonitor:
    case kInstMwait:
      // TODO: [COMPILER] Monitor/MWait.
      return NULL;

    case kInstPop:
      // TODO: [COMPILER] Pop.
      return NULL;

    // Not supported.
    case kInstPopa:
    case kInstPopf:
      return NULL;

    case kInstPush:
      // TODO: [COMPILER] Push.
      return NULL;

    // Not supported.
    case kInstPusha:
    case kInstPushf:
      return NULL;

    // Rot instruction is special only if the last operand is a variable.
    case kInstRcl:
    case kInstRcr:
    case kInstRol:
    case kInstRor:
    case kInstSal:
    case kInstSar:
    case kInstShl:
    case kInstShr:
      if (!opList[1].isVar())
        return NULL;
      return x86SpecialInstRot;

    // Shld/Shrd instruction is special only if the last operand is a variable.
    case kInstShld:
    case kInstShrd:
      if (!opList[2].isVar())
        return NULL;
      return x86SpecialInstShlrd;

    case kInstRdtsc:
    case kInstRdtscp:
      return x86SpecialInstRdtscRdtscp;

    case kInstRepLodsb:
    case kInstRepLodsd:
    case kInstRepLodsq:
    case kInstRepLodsw:
      return x86SpecialInstRepLod;

    case kInstRepMovsb:
    case kInstRepMovsd:
    case kInstRepMovsq:
    case kInstRepMovsw:
      return x86SpecialInstRepMovCmp;

    case kInstRepeCmpsb:
    case kInstRepeCmpsd:
    case kInstRepeCmpsq:
    case kInstRepeCmpsw:
      return x86SpecialInstRepMovCmp;

    case kInstRepneCmpsb:
    case kInstRepneCmpsd:
    case kInstRepneCmpsq:
    case kInstRepneCmpsw:
      return x86SpecialInstRepMovCmp;

    case kInstRepStosb:
    case kInstRepStosd:
    case kInstRepStosq:
    case kInstRepStosw:
      return x86SpecialInstRepSto;

    case kInstRepeScasb:
    case kInstRepeScasd:
    case kInstRepeScasq:
    case kInstRepeScasw:
      return x86SpecialInstRepSca;

    case kInstRepneScasb:
    case kInstRepneScasd:
    case kInstRepneScasq:
    case kInstRepneScasw:
      return x86SpecialInstRepSca;

    case kInstBlendvpd:
    case kInstBlendvps:
    case kInstPblendvb:
      return x86SpecialInstBlend;

    default:
      return NULL;
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitLoad]
// ============================================================================

void X86X64Context::emitLoad(VarData* vd, uint32_t regIndex, const char* reason) {
  ASMJIT_ASSERT(regIndex != kInvalidReg);

  X86X64Compiler* compiler = getCompiler();
  Mem m = getVarMem(vd);

  BaseNode* node = NULL;
  bool comment = _emitComments;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      node = compiler->emit(kInstMov, gpb_lo(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      node = compiler->emit(kInstMov, gpw(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kInstMov, gpd(regIndex), m);
      if (comment) goto _Comment;
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      node = compiler->emit(kInstMov, x64::gpq(regIndex), m);
      if (comment) goto _Comment;
      break;
#endif // ASMJIT_BUILD_X64

    case kVarTypeFp32:
    case kVarTypeFp64:
    case kVarTypeFpEx:
      // TODO: [COMPILER] FPU.
      break;

    case kVarTypeMm:
      node = compiler->emit(kInstMovq, mm(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeXmm:
      node = compiler->emit(kInstMovdqa, xmm(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSs:
      node = compiler->emit(kInstMovss, xmm(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSd:
      node = compiler->emit(kInstMovsd, xmm(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPs:
      node = compiler->emit(kInstMovaps, xmm(regIndex), m);
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPd:
      node = compiler->emit(kInstMovapd, xmm(regIndex), m);
      if (comment) goto _Comment;
      break;
  }
  return;

_Comment:
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitSave]
// ============================================================================

void X86X64Context::emitSave(VarData* vd, uint32_t regIndex, const char* reason) {
  ASMJIT_ASSERT(regIndex != kInvalidReg);

  X86X64Compiler* compiler = getCompiler();
  Mem m = getVarMem(vd);

  BaseNode* node = NULL;
  bool comment = _emitComments;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      node = compiler->emit(kInstMov, m, gpb_lo(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      node = compiler->emit(kInstMov, m, gpw(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kInstMov, m, gpd(regIndex));
      if (comment) goto _Comment;
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      node = compiler->emit(kInstMov, m, x64::gpq(regIndex));
      if (comment) goto _Comment;
      break;
#endif // ASMJIT_BUILD_X64

    case kVarTypeFp32:
    case kVarTypeFp64:
    case kVarTypeFpEx:
      // TODO: [COMPILER] FPU.
      break;

    case kVarTypeMm:
      node = compiler->emit(kInstMovq, m, mm(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmm:
      node = compiler->emit(kInstMovdqa, m, xmm(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSs:
      node = compiler->emit(kInstMovss, m, xmm(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSd:
      node = compiler->emit(kInstMovsd, m, xmm(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPs:
      node = compiler->emit(kInstMovaps, m, xmm(regIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPd:
      node = compiler->emit(kInstMovapd, m, xmm(regIndex));
      if (comment) goto _Comment;
      break;
  }
  return;

_Comment:
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitMove]
// ============================================================================

void X86X64Context::emitMove(VarData* vd, uint32_t toRegIndex, uint32_t fromRegIndex, const char* reason) {
  ASMJIT_ASSERT(toRegIndex   != kInvalidReg);
  ASMJIT_ASSERT(fromRegIndex != kInvalidReg);

  X86X64Compiler* compiler = getCompiler();

  BaseNode* node = NULL;
  bool comment = _emitComments;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
    case kVarTypeInt16:
    case kVarTypeUInt16:
    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kInstMov, gpd(toRegIndex), gpd(fromRegIndex));
      if (comment) goto _Comment;
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      node = compiler->emit(kInstMov, x64::gpq(toRegIndex), x64::gpq(fromRegIndex));
      if (comment) goto _Comment;
      break;
#endif // ASMJIT_BUILD_X64

    case kVarTypeFp32:
    case kVarTypeFp64:
    case kVarTypeFpEx:
      // TODO: [COMPILER] FPU.
      break;

    case kVarTypeMm:
      node = compiler->emit(kInstMovq, mm(toRegIndex), mm(fromRegIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmm:
      node = compiler->emit(kInstMovdqa, xmm(toRegIndex), xmm(fromRegIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSs:
      node = compiler->emit(kInstMovss, xmm(toRegIndex), xmm(fromRegIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmSd:
      node = compiler->emit(kInstMovsd, xmm(toRegIndex), xmm(fromRegIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPs:
      node = compiler->emit(kInstMovaps, xmm(toRegIndex), xmm(fromRegIndex));
      if (comment) goto _Comment;
      break;

    case kVarTypeXmmPd:
      node = compiler->emit(kInstMovapd, xmm(toRegIndex), xmm(fromRegIndex));
      if (comment) goto _Comment;
      break;
  }
  return;

_Comment:
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitSwap]
// ============================================================================

void X86X64Context::emitSwapGp(VarData* aVd, VarData* bVd, uint32_t aIndex, uint32_t bIndex, const char* reason) {
  ASMJIT_ASSERT(aIndex != kInvalidReg);
  ASMJIT_ASSERT(bIndex != kInvalidReg);

  X86X64Compiler* compiler = getCompiler();

  BaseNode* node = NULL;
  bool comment = _emitComments;

#if defined(ASMJIT_BUILD_X64)
  uint32_t vType = IntUtil::iMax(aVd->getType(), bVd->getType());

  if (vType == kVarTypeInt64 || vType == kVarTypeUInt64) {
    node = compiler->emit(kInstXchg, x64::gpq(aIndex), x64::gpq(bIndex));
    if (comment) goto _Comment;
    return;
  }
#endif // ASMJIT_BUILD_X64

  node = compiler->emit(kInstXchg, gpd(aIndex), gpd(bIndex));
  if (comment) goto _Comment;
  return;

_Comment:
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s, %s", reason, aVd->getName(), bVd->getName()));
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitPushSequence / EmitPopSequence]
// ============================================================================

void X86X64Context::emitPushSequence(uint32_t regs) {
  X86X64Compiler* compiler = getCompiler();
  uint32_t i = 0;

  GpReg gpReg(_zsp);
  while (regs != 0) {
    ASMJIT_ASSERT(i < _baseRegsCount);
    if ((regs & 0x1) != 0)
      compiler->emit(kInstPush, gpReg.setIndex(i));
    i++;
    regs >>= 1;
  }
}

void X86X64Context::emitPopSequence(uint32_t regs) {
  X86X64Compiler* compiler = getCompiler();
  int32_t i;
  uint32_t mask;

  if (regs == 0)
    return;

  GpReg gpReg(_zsp);
  for (i = _baseRegsCount - 1, mask = 0x1 << static_cast<uint32_t>(i); i >= 0; i--, mask >>= 1) {
    if ((regs & mask) == 0)
      continue;
    compiler->emit(kInstPop, gpReg.setIndex(i));
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitMoveArgOnStack / EmitMoveImmOnStack]
// ============================================================================

void X86X64Context::emitMoveVarOnStack(
  uint32_t dstType, const Mem* dst,
  uint32_t srcType, uint32_t srcIndex) {

  ASMJIT_ASSERT(srcIndex != kInvalidReg);
  X86X64Compiler* compiler = getCompiler();

  Mem m0(*dst);
  X86Reg r0;
  X86Reg r1;

  uint32_t regSize = compiler->getRegSize();
  uint32_t instCode;

  switch (dstType) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      // Move DWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeMm, kVarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeXmm, kVarTypeXmmPd))
        goto _MovXmmD;

      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      // Extend BYTE->WORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kRegTypeGpbLo, srcIndex);

        instCode = (dstType == kVarTypeInt16 && srcType == kVarTypeInt8) ? kInstMovsx : kInstMovzx;
        goto _ExtendMovGpD;
      }

      // Move DWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeMm, kVarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeXmm, kVarTypeXmmPd))
        goto _MovXmmD;

      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      // Extend BYTE->DWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kRegTypeGpbLo, srcIndex);

        instCode = (dstType == kVarTypeInt32 && srcType == kVarTypeInt8) ? kInstMovsx : kInstMovzx;
        goto _ExtendMovGpD;
      }

      // Extend WORD->DWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kRegTypeGpw, srcIndex);

        instCode = (dstType == kVarTypeInt32 && srcType == kVarTypeInt16) ? kInstMovsx : kInstMovzx;
        goto _ExtendMovGpD;
      }

      // Move DWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeMm, kVarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeXmm, kVarTypeXmmPd))
        goto _MovXmmD;
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
      // Extend BYTE->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kRegTypeGpbLo, srcIndex);

        instCode = (dstType == kVarTypeInt64 && srcType == kVarTypeInt8) ? kInstMovsx : kInstMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kRegTypeGpw, srcIndex);

        instCode = (dstType == kVarTypeInt64 && srcType == kVarTypeInt16) ? kInstMovsx : kInstMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt32)) {
        r1.setSize(4);
        r1.setCode(kRegTypeGpd, srcIndex);

        instCode = kInstMovsxd;
        if (dstType == kVarTypeInt64 && srcType == kVarTypeInt32)
          goto _ExtendMovGpXQ;
        else
          goto _ZeroExtendGpDQ;
      }

      // Move QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt64, kVarTypeUInt64))
        goto _MovGpQ;

      // Move QWORD (Mm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeMm, kVarTypeMm))
        goto _MovMmQ;

      // Move QWORD (Xmm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeXmm, kVarTypeXmmPd))
        goto _MovXmmQ;
      break;

    case kVarTypeMm:
      // Extend BYTE->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kRegTypeGpbLo, srcIndex);

        instCode = kInstMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kRegTypeGpw, srcIndex);

        instCode = kInstMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt32))
        goto _ExtendMovGpDQ;

      // Move QWORD (Gp).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeInt64, kVarTypeUInt64))
        goto _MovGpQ;

      // Move QWORD (Mm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeMm, kVarTypeMm))
        goto _MovMmQ;

      // Move QWORD (Xmm).
      if (IntUtil::inInterval<uint32_t>(srcType, kVarTypeXmm, kVarTypeXmmPd))
        goto _MovXmmQ;
      break;

    case kVarTypeXmm:
    case kVarTypeXmmPs:
    case kVarTypeXmmPd:
      // TODO: [COMPILER].
      break;

    case kVarTypeXmmSs:
      // TODO: [COMPILER].
      break;

    case kVarTypeXmmSd:
      // TODO: [COMPILER].
      break;
  }
  return;

  // Extend+Move Gp.
_ExtendMovGpD:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kRegTypeGpd, srcIndex);

  compiler->emit(instCode, r0, r1);
  compiler->emit(kInstMov, m0, r0);
  return;

_ExtendMovGpXQ:
  if (regSize == 8) {
    m0.setSize(8);
    r0.setSize(8);
    r0.setCode(kRegTypeGpq, srcIndex);

    compiler->emit(instCode, r0, r1);
    compiler->emit(kInstMov, m0, r0);
  }
  else {
    m0.setSize(4);
    r0.setSize(4);
    r0.setCode(kRegTypeGpd, srcIndex);

    compiler->emit(instCode, r0, r1);

_ExtendMovGpDQ:
    compiler->emit(kInstMov, m0, r0);
    m0.adjust(4);
    compiler->emit(kInstAnd, m0, 0);
  }
  return;

_ZeroExtendGpDQ:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kRegTypeGpd, srcIndex);
  goto _ExtendMovGpDQ;

  // Move Gp.
_MovGpD:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kRegTypeGpd, srcIndex);
  compiler->emit(kInstMov, m0, r0);
  return;

_MovGpQ:
  m0.setSize(8);
  r0.setSize(8);
  r0.setCode(kRegTypeGpq, srcIndex);
  compiler->emit(kInstMov, m0, r0);
  return;

  // Move Mm.
_MovMmD:
  m0.setSize(4);
  r0.setSize(8);
  r0.setCode(kRegTypeMm, srcIndex);
  compiler->emit(kInstMovd, m0, r0);
  return;

_MovMmQ:
  m0.setSize(8);
  r0.setSize(8);
  r0.setCode(kRegTypeMm, srcIndex);
  compiler->emit(kInstMovq, m0, r0);
  return;

  // Move Xmm.
_MovXmmD:
  m0.setSize(4);
  r0.setSize(16);
  r0.setCode(kRegTypeXmm, srcIndex);
  compiler->emit(kInstMovd, m0, r0);
  return;

_MovXmmQ:
  m0.setSize(8);
  r0.setSize(16);
  r0.setCode(kRegTypeXmm, srcIndex);
  compiler->emit(kInstMovq, m0, r0);
}

void X86X64Context::emitMoveImmOnStack(uint32_t dstType, const Mem* dst, const Imm* src) {
  X86X64Compiler* compiler = getCompiler();

  Mem mem(*dst);
  Imm imm(*src);

  uint32_t regSize = compiler->getRegSize();

  // One stack entry is equal to the native register size. That means that if
  // we want to move 32-bit integer on the stack, we need to extend it to 64-bit
  // integer.
  mem.setSize(regSize);

  switch (dstType) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      imm.truncateTo8Bits();
      compiler->emit(kInstMov, mem, imm);
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      imm.truncateTo16Bits();
      compiler->emit(kInstMov, mem, imm);
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
_Move32:
      imm.truncateTo32Bits();
      compiler->emit(kInstMov, mem, imm);
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
_Move64:
      if (regSize == 4) {
        uint32_t hi = imm.getUInt32Hi();

        // Lo-Part.
        imm.truncateTo32Bits();
        compiler->emit(kInstMov, mem, imm);

        // Hi-Part.
        mem.adjust(regSize);
        imm.setUInt32(hi);
        compiler->emit(kInstMov, mem, imm);
      }
      else {
        compiler->emit(kInstMov, mem, imm);
      }
      break;

    case kVarTypeFp32:
      goto _Move32;

    case kVarTypeFp64:
      goto _Move64;

    case kVarTypeFpEx:
      // Not supported.
      ASMJIT_ASSERT(!"Reached");
      break;

    case kVarTypeMm:
      goto _Move64;

    case kVarTypeXmm:
    case kVarTypeXmmSs:
    case kVarTypeXmmPs:
    case kVarTypeXmmSd:
    case kVarTypeXmmPd:
      if (regSize == 4) {
        uint32_t hi = imm.getUInt32Hi();

        // Lo-Part.
        imm.truncateTo32Bits();
        compiler->emit(kInstMov, mem, imm);

        // Hi-Part.
        mem.adjust(regSize);
        imm.setUInt32(hi);
        compiler->emit(kInstMov, mem, imm);

        // Zero part - performing AND should generate shorter code, because
        // 8-bit immediate can be used instead of 32-bit immediate required
        // by MOV instruction.
        mem.adjust(regSize);
        imm.setUInt32(0);
        compiler->emit(kInstAnd, mem, imm);

        mem.adjust(regSize);
        compiler->emit(kInstAnd, mem, imm);
      }
      else {
        // Lo-Hi parts.
        compiler->emit(kInstMov, mem, imm);

        // Zero part.
        mem.adjust(regSize);
        imm.setUInt32(0);
        compiler->emit(kInstAnd, mem, imm);
      }
      break;

    default:
      ASMJIT_ASSERT(!"Reached");
      break;
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - EmitMoveImmToReg]
// ============================================================================

void X86X64Context::emitMoveImmToReg(uint32_t dstType, uint32_t dstIndex, const Imm* src) {
  ASMJIT_ASSERT(dstIndex != kInvalidReg);
  X86X64Compiler* compiler = getCompiler();

  X86Reg r0;
  Imm imm(*src);

  switch (dstType) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      imm.truncateTo8Bits();
      goto _Move32;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      imm.truncateTo16Bits();
      goto _Move32;

    case kVarTypeInt32:
    case kVarTypeUInt32:
_Move32Truncate:
      imm.truncateTo32Bits();
_Move32:
      r0.setSize(4);
      r0.setCode(kRegTypeGpd, dstIndex);
      compiler->emit(kInstMov, r0, imm);
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
      // Move to GPD register will clear the HI-DWORD of GPQ register in 64-bit
      // mode.
      if (imm.isUInt32())
        goto _Move32Truncate;

      r0.setSize(8);
      r0.setCode(kRegTypeGpq, dstIndex);
      compiler->emit(kInstMov, r0, imm);
      break;

    case kVarTypeFp32:
    case kVarTypeFp64:
    case kVarTypeFpEx:
      // TODO: [COMPILER] EmitMoveImmToReg.
      break;

    case kVarTypeMm:
      // TODO: [COMPILER] EmitMoveImmToReg.
      break;

    case kVarTypeXmm:
    case kVarTypeXmmSs:
    case kVarTypeXmmSd:
    case kVarTypeXmmPs:
    case kVarTypeXmmPd:
      // TODO: [COMPILER] EmitMoveImmToReg.
      break;
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Register Management]
// ============================================================================

#if defined(ASMJIT_DEBUG)
template<int C>
static ASMJIT_INLINE void X86X64Context_checkStateVars(X86X64Context* self) {
  VarState* state = self->getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t regIndex;
  uint32_t regMask;
  uint32_t regCount = self->getRegsCount(C);

  uint32_t occupied = state->_occupied.get(C);
  uint32_t modified = state->_modified.get(C);

  for (regIndex = 0, regMask = 1; regIndex < regCount; regIndex++, regMask <<= 1) {
    VarData* vd = sVars[regIndex];

    if (vd == NULL) {
      ASMJIT_ASSERT((occupied & regMask) == 0);
      ASMJIT_ASSERT((modified & regMask) == 0);
    }
    else {
      ASMJIT_ASSERT((occupied & regMask) != 0);
      ASMJIT_ASSERT((modified & regMask) == (static_cast<uint32_t>(vd->isModified()) << regIndex));

      ASMJIT_ASSERT(vd->getClass() == C);
      ASMJIT_ASSERT(vd->getState() == kVarStateReg);
      ASMJIT_ASSERT(vd->getRegIndex() == regIndex);
    }
  }
}

void X86X64Context::_checkState() {
  X86X64Context_checkStateVars<kRegClassGp>(this);
  X86X64Context_checkStateVars<kRegClassMm>(this);
  X86X64Context_checkStateVars<kRegClassXy>(this);
}
#else
void X86X64Context::_checkState() {}
#endif // ASMJIT_DEBUG

// ============================================================================
// [asmjit::x86x64::X86X64Context - State - Load]
// ============================================================================

template<int C>
static ASMJIT_INLINE void X86X64Context_loadStateVars(X86X64Context* self, VarState* target) {
  VarState* state = self->getState();

  VarData** sVars = state->getListByClass(C);
  VarData** tVars = target->getListByClass(C);

  uint32_t regIndex;
  uint32_t modified = target->_modified.get(C);
  uint32_t regCount = self->getRegsCount(C);

  for (regIndex = 0; regIndex < regCount; regIndex++, modified >>= 1) {
    VarData* vd = tVars[regIndex];
    sVars[regIndex] = vd;

    if (vd == NULL)
      continue;

    vd->setState(kVarStateReg);
    vd->setRegIndex(regIndex);
    vd->setModified(modified & 0x1);
  }
}

void X86X64Context::loadState(BaseVarState* target_) {
  VarState* state = getState();
  VarState* target = static_cast<VarState*>(target_);

  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  // Load allocated variables.
  X86X64Context_loadStateVars<kRegClassGp>(this, target);
  X86X64Context_loadStateVars<kRegClassMm>(this, target);
  X86X64Context_loadStateVars<kRegClassXy>(this, target);

  // Load masks.
  state->_occupied = target->_occupied;
  state->_modified = target->_modified;

  // Load states of other variables and clear their 'Modified' flags.
  for (uint32_t i = 0; i < vdCount; i++) {
    uint32_t vState = target->_cells[i].getState();

    if (vState != kVarStateReg) {
      vdArray[i]->setState(vState);
      vdArray[i]->setModified(false);
    }
  }

  ASMJIT_CONTEXT_CHECK_STATE
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - State - Save]
// ============================================================================

BaseVarState* X86X64Context::saveState() {
  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  size_t size = IntUtil::alignTo<size_t>(
    sizeof(VarState) + vdCount * sizeof(StateCell), sizeof(void*));

  VarState* cur = getState();
  VarState* dst = _zoneAllocator.allocT<VarState>(size);

  if (dst == NULL)
    return NULL;

  // Store links.
  ::memcpy(dst->_list, cur->_list, VarState::kAllCount * sizeof(VarData*));

  // Store masks.
  dst->_occupied = cur->_occupied;
  dst->_modified = cur->_modified;

  // Store cells.
  for (uint32_t i = 0; i < vdCount; i++) {
    VarData* vd = static_cast<VarData*>(vdArray[i]);
    StateCell& cell = dst->_cells[i];

    cell.reset();
    cell.setState(vd->getState());
  }

  return dst;
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - State - Switch]
// ============================================================================

template<int C>
static ASMJIT_INLINE void X86X64Context_switchStateVars(X86X64Context* self, VarState* src) {
  VarState* dst = self->getState();

  VarData** dstVars = dst->getListByClass(C);
  VarData** srcVars = src->getListByClass(C);

  uint32_t regIndex;
  uint32_t regMask;
  uint32_t regCount = self->getRegsCount(C);

  StateCell* cells = src->_cells;

  bool didWork;
  do {
    didWork = false;

    for (regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* dVd = dstVars[regIndex];
      VarData* sVd = srcVars[regIndex];

      if (dVd == sVd)
        continue;

      if (dVd != NULL) {
        StateCell& cell = cells[dVd->getContextId()];

        if (cell.getState() != kVarStateReg) {
          if (cell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);

          dVd = NULL;
          didWork = true;

          if (sVd == NULL)
            continue;
        }
      }

      if (dVd == NULL && sVd != NULL) {
        if (sVd->getRegIndex() != kInvalidReg)
          self->move<C>(sVd, regIndex);
        else
          self->load<C>(sVd, regIndex);

        didWork = true;
        continue;
      }

      if (dVd != NULL && sVd == NULL) {
        StateCell& cell = cells[dVd->getContextId()];
        if (cell.getState() == kVarStateReg)
          continue;

        if (cell.getState() == kVarStateMem)
          self->spill<C>(dVd);
        else
          self->unuse<C>(dVd);

        didWork = true;
        continue;
      }

      if (C == kRegClassGp) {
        self->swapGp(dVd, sVd);

        didWork = true;
        continue;
      }
      else {
        self->spill<C>(dVd);
        self->move<C>(sVd, regIndex);

        didWork = true;
        continue;
      }
    }
  } while (didWork);

  uint32_t dstModified = dst->_modified.get(C);
  uint32_t srcModified = src->_modified.get(C);

  if (dstModified != srcModified) {
    for (regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* vd = dstVars[regIndex];

      if (vd == NULL)
        continue;

      if ((dstModified & regMask) && !(srcModified & regMask))
        self->save<C>(vd);
      else if (!(dstModified & regMask) && (srcModified & regMask))
        self->modify<C>(vd);
    }
  }
}

void X86X64Context::switchState(BaseVarState* src_) {
  VarState* cur = getState();
  VarState* src = static_cast<VarState*>(src_);

  // Ignore if both states are equal.
  if (cur == src)
    return;

  // Switch variables.
  X86X64Context_switchStateVars<kRegClassGp>(this, src);
  X86X64Context_switchStateVars<kRegClassMm>(this, src);
  X86X64Context_switchStateVars<kRegClassXy>(this, src);

  // Copy occupied mask.
  // TODO: Review.
  // cur->_occupied = src->_occupied;
  // cur->_modified = src->_modified;

  // Calculate changed state.
  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  StateCell* cells = src->_cells;
  for (uint32_t i = 0; i < vdCount; i++) {
    VarData* vd = static_cast<VarData*>(vdArray[i]);
    StateCell& cell = cells[i];

    uint32_t vState = cell.getState();
    if (vState != kVarStateReg) {
      vd->setState(vState);
      vd->setModified(false);
    }
  }

  ASMJIT_CONTEXT_CHECK_STATE
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - State - Intersect]
// ============================================================================

void X86X64Context::intersectStates(BaseVarState* a_, BaseVarState* b_) {
  VarState* aState = static_cast<VarState*>(a_);
  VarState* bState = static_cast<VarState*>(b_);

  // TODO: [COMPILER] Intersect states.

  ASMJIT_CONTEXT_CHECK_STATE
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - GetJccFlow / GetOppositeJccFlow]
// ============================================================================

//! @internal
static ASMJIT_INLINE BaseNode* X86X64Context_getJccFlow(JumpNode* jNode) {
  if (jNode->isTaken())
    return jNode->getTarget();
  else
    return jNode->getNext();
}

//! @internal
static ASMJIT_INLINE BaseNode* X86X64Context_getOppositeJccFlow(JumpNode* jNode) {
  if (jNode->isTaken())
    return jNode->getNext();
  else
    return jNode->getTarget();
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Prepare - SingleVarInst]
// ============================================================================

//! @internal
static void X86X64Context_prepareSingleVarInst(uint32_t code, VarAttr* va) {
  switch (code) {
    // - andn     reg, reg ; Set all bits in reg to 0.
    // - xor/pxor reg, reg ; Set all bits in reg to 0.
    // - sub/psub reg, reg ; Set all bits in reg to 0.
    // - pcmpgt   reg, reg ; Set all bits in reg to 0.
    // - pcmpeq   reg, reg ; Set all bits in reg to 1.
    case kInstPandn     :
    case kInstXor       : case kInstXorpd     : case kInstXorps     : case kInstPxor      :
    case kInstSub:
    case kInstPsubb     : case kInstPsubw     : case kInstPsubd     : case kInstPsubq     :
    case kInstPsubsb    : case kInstPsubsw    : case kInstPsubusb   : case kInstPsubusw   :
    case kInstPcmpeqb   : case kInstPcmpeqw   : case kInstPcmpeqd   : case kInstPcmpeqq   :
    case kInstPcmpgtb   : case kInstPcmpgtw   : case kInstPcmpgtd   : case kInstPcmpgtq   :
      va->delFlags(kVarAttrInReg);
      break;

    // - and      reg, reg ; Nop.
    // - or       reg, reg ; Nop.
    // - xchg     reg, reg ; Nop.
    case kInstAnd       : case kInstAndpd     : case kInstAndps     : case kInstPand      :
    case kInstOr        : case kInstOrpd      : case kInstOrps      : case kInstPor       :
    case kInstXchg      :
      va->delFlags(kVarAttrOutReg);
      break;
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Prepare]
// ============================================================================

//! @internal
//!
//! @brief Add unreachable-flow data to the unreachable flow list.
static ASMJIT_INLINE Error X86X64Context_prepareAddUnreachableNode(X86X64Context* self, BaseNode* node) {
  PodList<BaseNode*>::Link* link = self->_zoneAllocator.allocT<PodList<BaseNode*>::Link>();
  if (link == NULL)
    return self->setError(kErrorNoHeapMemory);

  link->setValue(node);
  self->_unreachableList.append(link);

  return kErrorOk;
}

//! @internal
//!
//! @brief Add jump-flow data to the jcc flow list.
static ASMJIT_INLINE Error X86X64Context_prepareAddJccNode(X86X64Context* self, BaseNode* node) {
  PodList<BaseNode*>::Link* link = self->_zoneAllocator.allocT<PodList<BaseNode*>::Link>();

  if (link == NULL)
    ASMJIT_PROPAGATE_ERROR(self->setError(kErrorNoHeapMemory));

  link->setValue(node);
  self->_jccList.append(link);

  return kErrorOk;
}

//! @internal
//!
//! @brief Get mask of all registers actually used to pass function arguments.
static ASMJIT_INLINE RegMask X86X64Context_getUsedArgs(X86X64Context* self, X86X64CallNode* node, X86X64FuncDecl* decl) {
  RegMask regs;
  regs.reset();

  uint32_t i;
  uint32_t argCount = decl->getArgCount();

  for (i = 0; i < argCount; i++) {
    const FuncInOut& arg = decl->getArg(i);
    if (!arg.hasRegIndex())
      continue;
    regs.add(x86VarTypeToClass(arg.getVarType()), IntUtil::mask(arg.getRegIndex()));
  }

  return regs;
}

// ============================================================================
// [Helpers]
// ============================================================================

static ASMJIT_INLINE SArgNode* X86X64Context_insertSArgNode(
  X86X64Context* self,
  X86X64Compiler* compiler,
  X86X64CallNode* call,
  VarData* vd, const uint32_t* gaRegs) {

  uint32_t vType = vd->getType();
  const VarInfo& vInfo = _varInfo[vType];
  uint32_t c = vInfo.getClass();

  SArgNode* sArg = compiler->newNode<SArgNode>(vd, call);
  if (sArg == NULL)
    return NULL;

  VarInst* vi = self->newVarInst(1);
  if (vi == NULL)
    return NULL;

  vi->_vaCount = 1;
  vi->_count.reset();
  vi->_count.add(c);
  vi->_start.reset();
  vi->_inRegs.reset();
  vi->_outRegs.reset();
  vi->_clobberedRegs.reset();
  vi->_list[0].setup(vd, kVarAttrInReg, 0, gaRegs[c]);

  sArg->setVarInst(vi);

  compiler->addNodeBefore(sArg, call);
  return sArg;
}

//! @internal
//!
//! @brief Prepare the given function @a func.
//!
//! For each node:
//! - Create and assign groupId and flowId.
//! - Collect all variables and merge them to vaList.
Error X86X64Context::fetch() {
  X86X64Compiler* compiler = getCompiler();
  X86X64FuncNode* func = getFunc();

  uint32_t arch = compiler->getArch();

  BaseNode* node_ = func;
  BaseNode* next = NULL;
  BaseNode* stop = getStop();

  uint32_t groupId = 1;
  uint32_t flowId = 0;

  VarAttr vaTmpList[80];
  PodList<BaseNode*>::Link* jLink = NULL;

  // Function flags.
  func->clearFuncFlags(
    kFuncFlagIsNaked |
    kFuncFlagPushPop |
    kFuncFlagEmms    |
    kFuncFlagSFence  |
    kFuncFlagLFence  );

  if (func->getHint(kFuncHintNaked  ) != 0) func->addFuncFlags(kFuncFlagIsNaked);
  if (func->getHint(kFuncHintCompact) != 0) func->addFuncFlags(kFuncFlagPushPop | kFuncFlagEnter | kFuncFlagLeave);
  if (func->getHint(kFuncHintPushPop) != 0) func->addFuncFlags(kFuncFlagPushPop);
  if (func->getHint(kFuncHintEmms   ) != 0) func->addFuncFlags(kFuncFlagEmms   );
  if (func->getHint(kFuncHintSFence ) != 0) func->addFuncFlags(kFuncFlagSFence );
  if (func->getHint(kFuncHintLFence ) != 0) func->addFuncFlags(kFuncFlagLFence );

  // Global allocable registers.
  uint32_t* gaRegs = _gaRegs;

  if (!func->hasFuncFlag(kFuncFlagIsNaked))
    gaRegs[kRegClassGp] &= ~IntUtil::mask(kRegIndexBp);

  // Allowed index registers (Gp/Xmm/Ymm).
  const uint32_t indexMask = IntUtil::bits(_baseRegsCount) & ~(IntUtil::mask(4, 12));

  // --------------------------------------------------------------------------
  // [VI Macros]
  // --------------------------------------------------------------------------

#define VI_BEGIN() \
  do { \
    uint32_t vaCount = 0; \
    RegCount regCount; \
    \
    RegMask inRegs; \
    RegMask outRegs; \
    RegMask clobberedRegs; \
    \
    regCount.reset(); \
    inRegs.reset(); \
    outRegs.reset(); \
    clobberedRegs.reset()

#define VI_END(_Node_) \
    if (vaCount == 0 && clobberedRegs.isEmpty()) \
      break; \
    \
    VarInst* vi = newVarInst(vaCount); \
    if (vi == NULL) \
      goto _NoMemory; \
    \
    RegCount vaIndex; \
    vaIndex.makeIndex(regCount); \
    \
    vi->_vaCount = vaCount; \
    vi->_count = regCount; \
    vi->_start = vaIndex; \
    \
    vi->_inRegs = inRegs; \
    vi->_outRegs = outRegs; \
    vi->_clobberedRegs = clobberedRegs; \
    \
    VarAttr* va = vaTmpList; \
    while (vaCount) { \
      VarData* vd = va->getVd(); \
      \
      uint32_t class_ = vd->getClass(); \
      uint32_t index = vaIndex.get(class_); \
      \
      vaIndex.add(class_); \
      \
      if (va->_inRegs) \
        va->_allocableRegs = va->_inRegs; \
      else if (va->_outRegIndex != kInvalidReg) \
        va->_allocableRegs = IntUtil::mask(va->_outRegIndex); \
      else \
        va->_allocableRegs &= ~inRegs._regs[class_]; \
      \
      vd->_va = NULL; \
      vi->getVa(index)[0] = va[0]; \
      \
      va++; \
      vaCount--; \
    } \
    \
    _Node_->setVarInst(vi); \
  } while (0)

#define VI_UPDATE_CID(_Vd_) \
  do { \
    if (!_Vd_->hasContextId()) { \
      _Vd_->setContextId(static_cast<uint32_t>(_contextVd.getLength())); \
      if (_contextVd.append(_Vd_) != kErrorOk) \
        goto _NoMemory; \
    } \
  } while (0)

#define VI_ADD_VAR(_Vd_, _Va_, _Flags_, _NewAllocable_) \
  do { \
    ASMJIT_ASSERT(_Vd_->_va == NULL); \
    \
    _Va_ = &vaTmpList[vaCount++]; \
    _Va_->setup(_Vd_, _Flags_, 0, _NewAllocable_); \
    _Va_->addVarCount(1); \
    _Vd_->setVa(_Va_); \
    \
    VI_UPDATE_CID(_Vd_); \
    regCount.add(_Vd_->getClass()); \
  } while (0)

#define VI_MERGE_VAR(_Vd_, _Va_, _Flags_, _NewAllocable_) \
  do { \
    _Va_ = _Vd_->getVa(); \
    \
    if (_Va_ == NULL) { \
      _Va_ = &vaTmpList[vaCount++]; \
      _Va_->setup(_Vd_, 0, 0, _NewAllocable_); \
      _Vd_->setVa(_Va_); \
      \
      VI_UPDATE_CID(_Vd_); \
      regCount.add(_Vd_->getClass()); \
    } \
    \
    _Va_->addFlags(_Flags_); \
    _Va_->addVarCount(1); \
  } while (0)

  // --------------------------------------------------------------------------
  // [Loop]
  // --------------------------------------------------------------------------

  do {
_Do:
    while (node_->isFetched()) {
_NextGroup:
      if (jLink == NULL)
        jLink = _jccList.getFirst();
      else
        jLink = jLink->getNext();

      if (jLink == NULL)
        goto _Done;
      node_ = X86X64Context_getOppositeJccFlow(static_cast<JumpNode*>(jLink->getValue()));
    }

    flowId++;

    next = node_->getNext();
    node_->setFlowId(flowId);

    switch (node_->getType()) {
      // ----------------------------------------------------------------------
      // [Align/Embed]
      // ----------------------------------------------------------------------

      case kNodeTypeAlign:
      case kNodeTypeEmbed:
        break;

      // ----------------------------------------------------------------------
      // [Hint]
      // ----------------------------------------------------------------------

      case kNodeTypeHint: {
        HintNode* node = static_cast<HintNode*>(node_);
        VI_BEGIN();

        if (node->getHint() == kVarHintAlloc) {
          HintNode* cur = node;

          uint32_t remain[kRegClassCount];
          RegMask inRegs;

          remain[kRegClassGp] = _baseRegsCount - 1 - func->hasFuncFlag(kFuncFlagIsNaked);
          remain[kRegClassFp] = kRegCountFp;
          remain[kRegClassMm] = kRegCountMm;
          remain[kRegClassXy] = _baseRegsCount;
          inRegs.reset();

          // Merge as many alloc-hints as possible.
          for (;;) {
            VarData* vd = static_cast<VarData*>(cur->getVd());
            VarAttr* va = vd->getVa();

            uint32_t regClass = vd->getClass();
            uint32_t regIndex = cur->getValue();
            uint32_t regMask = 0;

            // We handle both kInvalidReg and kInvalidValue.
            if (regIndex < kInvalidReg)
              regMask = IntUtil::mask(regIndex);

            if (va == NULL) {
              if ((inRegs._regs[regClass] & regMask) != 0)
                break;
              if (remain[regClass] == 0)
                break;
              VI_ADD_VAR(vd, va, kVarAttrInReg, gaRegs[regClass]);

              if (regMask != 0) {
                inRegs._regs[regClass] ^= static_cast<uint16_t>(regMask);
                va->setInRegs(regMask);
                va->setInRegIndex(regIndex);
              }

              remain[regClass]--;
            }
            else if (regMask != 0) {
              if ((inRegs._regs[regClass] & regMask) != 0 && va->getInRegs() != regMask)
                break;

              inRegs._regs[regClass] ^= static_cast<uint16_t>(va->getInRegs() | regMask);
              va->setInRegs(regMask);
              va->setInRegIndex(regIndex);
            }

            if (cur != node)
              compiler->removeNode(cur);

            cur = static_cast<HintNode*>(node->getNext());
            if (cur == NULL || cur->getType() != kNodeTypeHint || cur->getHint() != kVarHintAlloc)
              break;
          }

          next = node->getNext();
        }
        else  {
          VarData* vd = static_cast<VarData*>(node->getVd());
          VarAttr* va;

          uint32_t flags = 0;

          switch (node->getHint()) {
            case kVarHintSpill:
              flags = kVarAttrInMem;
              break;
            case kVarHintSave:
              flags = kVarAttrInMem;
              break;
            case kVarHintSaveAndUnuse:
              flags = kVarAttrInMem | kVarAttrUnuse;
              break;
            case kVarHintUnuse:
              flags = kVarAttrUnuse;
              break;
          }

          VI_ADD_VAR(vd, va, flags, 0);
        }

        VI_END(node_);
        break;
      }

      // ----------------------------------------------------------------------
      // [Target]
      // ----------------------------------------------------------------------

      case kNodeTypeTarget: {
        break;
      }

      // ----------------------------------------------------------------------
      // [Inst]
      // ----------------------------------------------------------------------

      case kNodeTypeInst: {
        InstNode* node = static_cast<InstNode*>(node_);

        uint32_t code = node->getCode();
        uint32_t flags = node->getFlags();

        Operand* opList = node->getOpList();
        uint32_t opCount = node->getOpCount();

        if (opCount) {
          const InstInfo* info = &_instInfo[code];
          const X86X64SpecialInst* special = NULL;
          VI_BEGIN();

          // Collect instruction flags and merge all 'VarAttr's.
          if (info->isFp())
            flags |= kNodeFlagIsFp;

          if (info->isSpecial() && (special = X86X64SpecialInst_get(code, opList, opCount)) != NULL)
            flags |= kNodeFlagIsSpecial;

          uint32_t gpAllowedMask = 0xFFFFFFFF;

          for (uint32_t i = 0; i < opCount; i++) {
            Operand* op = &opList[i];
            VarData* vd;
            VarAttr* va;

            if (op->isVar()) {
              vd = compiler->getVdById(op->getId());
              VI_MERGE_VAR(vd, va, 0, gaRegs[vd->getClass()] & gpAllowedMask);

              if (static_cast<X86Var*>(op)->isGpb()) {
                va->addFlags(static_cast<GpVar*>(op)->isGpbLo() ? kVarAttrGpbLo : kVarAttrGpbHi);
                if (arch == kArchX86) {
                  // If a byte register is accessed in 32-bit mode we have to limit
                  // all allocable registers for that variable to eax/ebx/ecx/edx.
                  // Other variables are not affected.
                  va->_allocableRegs &= 0x0F;
                }
                else {
                  // It's fine if lo-byte register is accessed in 64-bit mode;
                  // however, hi-byte has to be checked and if it's used all
                  // registers (Gp/Xmm) could be only allocated in the lower eight
                  // half. To do that, we patch 'allocableRegs' of all variables
                  // we collected until now and change the allocable restriction
                  // for variables that come after.
                  if (static_cast<GpVar*>(op)->isGpbHi()) {
                    va->_allocableRegs &= 0x0F;

                    if (gpAllowedMask != 0xFF) {
                      for (uint32_t j = 0; j < i; j++)
                        vaTmpList[j]._allocableRegs &= vaTmpList[j].hasFlag(kVarAttrGpbHi) ? 0x0F : 0xFF;
                      gpAllowedMask = 0xFF;
                    }
                  }
                }
              }

              if (special != NULL) {
                uint32_t inReg = special[i].inReg;
                uint32_t outReg = special[i].outReg;
                uint32_t c;

                if (static_cast<const X86Reg*>(op)->isGp())
                  c = kRegClassGp;
                else
                  c = kRegClassXy;

                if (inReg != kInvalidReg) {
                  uint32_t mask = IntUtil::mask(inReg);
                  inRegs.add(c, mask);
                  va->addInRegs(mask);
                }

                if (outReg != kInvalidReg) {
                  uint32_t mask = IntUtil::mask(outReg);
                  outRegs.add(c, mask);
                  va->setOutRegIndex(outReg);
                }

                va->addFlags(special[i].flags);
              }
              else {
                uint32_t inFlags = kVarAttrInReg;
                uint32_t outFlags = kVarAttrOutReg;
                uint32_t combinedFlags;

                if (i == 0) {
                  // Default for the first operand.
                  combinedFlags = inFlags | outFlags;

                  // Comparison/Test instructions never modify the source operand.
                  if (info->isTest()) {
                    combinedFlags = inFlags;
                  }
                  // Move instructions typically overwrite the first operand, but
                  // there are some exceptions based on the operands' size and type.
                  else if (info->isMove()) {
                    // Cvttsd2si/Cvttss2si. In 32-bit mode the whole destination is replaced.
                    // In 64-bit mode we need to check whether the destination operand size
                    // is 64-bits.
                    if (code == kInstCvttsd2si || code == kInstCvttss2si)
                      combinedFlags = vd->getSize() > 4 ? (op->isRegType(kRegTypeGpq) ? outFlags : inFlags | outFlags) : outFlags;
                    // Movss/Movsd. These instructions won't overwrite the whole register if move
                    // is between two registers.
                    else if (code == kInstMovss || code == kInstMovsd)
                      combinedFlags = opList[1].isMem() ? outFlags : inFlags | outFlags;
                    else
                      combinedFlags = outFlags;
                  }
                  // Imul.
                  else if (code == kInstImul && opCount == 3) {
                    combinedFlags = outFlags;
                  }
                }
                else {
                  // Default for secon/third operands.
                  combinedFlags = inFlags;

                  // Xchg/Xadd/Imul/Idiv.
                  if (info->isXchg() || (code == kInstImul && opCount == 3 && i == 1))
                    combinedFlags = inFlags | outFlags;
                }
                va->addFlags(combinedFlags);
              }
            }
            else if (op->isMem()) {
              Mem* m = static_cast<Mem*>(op);
              node->setMemOpIndex(i);

              if (OperandUtil::isVarId(m->getBase()) && m->isBaseIndexType()) {
                vd = compiler->getVdById(m->getBase());
                if (!vd->isStack()) {
                  VI_MERGE_VAR(vd, va, 0, gaRegs[vd->getClass()] & gpAllowedMask);
                  if (m->getMemType() == kMemTypeBaseIndex) {
                    va->addFlags(kVarAttrInReg);
                  }
                  else {
                    uint32_t inFlags = kVarAttrInMem;
                    uint32_t outFlags = kVarAttrOutMem;
                    uint32_t combinedFlags;

                    if (i == 0) {
                      // Default for the first operand.
                      combinedFlags = inFlags | outFlags;

                      // Comparison/Test instructions never modify the source operand.
                      if (info->isTest()) {
                        combinedFlags = inFlags;
                      }
                      // Move instructions typically overwrite the first operand, but
                      // there are some exceptions based on the operands' size and type.
                      else if (info->isMove()) {
                        // Movss.
                        if (code == kInstMovss)
                          combinedFlags = vd->getSize() == 4 ? outFlags : inFlags | outFlags;
                        // Movsd.
                        else if (code == kInstMovsd)
                          combinedFlags = vd->getSize() == 8 ? outFlags : inFlags | outFlags;
                        else
                          combinedFlags = outFlags;
                      }
                    }
                    else {
                      // Default for the second operand.
                      combinedFlags = inFlags;
                      if (info->isXchg())
                        combinedFlags = inFlags | outFlags;
                    }

                    va->addFlags(combinedFlags);
                  }
                }
              }

              if (OperandUtil::isVarId(m->getIndex())) {
                // Restrict allocation to all registers except ESP/RSP/R12.
                vd = compiler->getVdById(m->getIndex());
                VI_MERGE_VAR(vd, va, 0, gaRegs[kRegClassGp] & gpAllowedMask);
                va->andAllocableRegs(indexMask);
                va->addFlags(kVarAttrInReg);
              }
            }
          }

          node->setFlags(flags);
          if (vaCount) {
            // Handle instructions which result in zeros/ones or nop if used with the
            // same destination and source operand.
            if (vaCount == 1 && opCount >= 2 && opList[0].isVar() && opList[1].isVar() && !node->hasMemOp())
              X86X64Context_prepareSingleVarInst(code, &vaTmpList[0]);
          }

          VI_END(node_);
        }

        // Handle conditional/unconditional jump.
        if (node->isJmpOrJcc()) {
          JumpNode* jNode = static_cast<JumpNode*>(node);

          BaseNode* jNext = jNode->getNext();
          TargetNode* jTarget = jNode->getTarget();

          // If this jump is unconditional we put next node to unreachable node
          // list so we can eliminate possible dead code. We have to do this in
          // all cases since we are unable to translate without fetch() step.
          //
          // We also advance our node pointer to the target node to simulate
          // natural flow of the function.
          if (jNode->isJmp()) {
            if (!jNext->isFetched())
              ASMJIT_PROPAGATE_ERROR(X86X64Context_prepareAddUnreachableNode(this, jNext));

            node_ = jTarget;
            goto _Do;
          }
          else {
            if (jTarget->isFetched()) {
              uint32_t jTargetFlowId = jTarget->getFlowId();

              // Update kNodeFlagIsTaken flag to true if this is a conditional
              // backward jump. This behavior can be overridden by using
              // kCondHintUnlikely when the instruction is created.
              if (!jNode->isTaken() && opCount == 1 && jTargetFlowId <= flowId) {
                jNode->addFlags(kNodeFlagIsTaken);
              }
            }
            else if (jNext->isFetched()) {
              node_ = jTarget;
              goto _Do;
            }
            else {
              ASMJIT_PROPAGATE_ERROR(X86X64Context_prepareAddJccNode(this, jNode));

              node_ = X86X64Context_getJccFlow(jNode);
              goto _Do;
            }
          }
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Func]
      // ----------------------------------------------------------------------

      case kNodeTypeFunc: {
        ASMJIT_ASSERT(node_ == func);
        X86X64FuncDecl* decl = func->getDecl();

        VI_BEGIN();
        for (uint32_t i = 0, argCount = decl->getArgCount(); i < argCount; i++) {
          const FuncInOut& arg = decl->getArg(i);

          VarData* vd = func->getArg(i);
          VarAttr* va;

          if (vd == NULL)
            continue;

          // Overlapped function arguments.
          if (vd->getVa() != NULL)
            return compiler->setError(kErrorCompilerOverlappedArgs);
          VI_ADD_VAR(vd, va, 0, 0);

          if (x86VarTypeToClass(arg.getVarType()) == vd->getClass()) {
            if (arg.hasRegIndex()) {
              va->addFlags(kVarAttrOutReg);
              va->setOutRegIndex(arg.getRegIndex());
            }
            else {
              va->addFlags(kVarAttrOutMem);
            }
          }
          else {
            // TODO: [COMPILER] Function Argument Conversion.
            va->addFlags(kVarAttrOutDecide | kVarAttrOutConv);
          }
        }
        VI_END(node_);
        break;
      }

      // ----------------------------------------------------------------------
      // [End]
      // ----------------------------------------------------------------------

      case kNodeTypeEnd: {
        goto _NextGroup;
      }

      // ----------------------------------------------------------------------
      // [Ret]
      // ----------------------------------------------------------------------

      case kNodeTypeRet: {
        RetNode* node = static_cast<RetNode*>(node_);
        X86X64FuncDecl* decl = func->getDecl();

        if (decl->hasRet()) {
          const FuncInOut& ret = decl->getRet(0);
          uint32_t retClass = x86VarTypeToClass(ret.getVarType());

          VI_BEGIN();
          for (uint32_t i = 0; i < 2; i++) {
            Operand* op = &node->_ret[i];

            if (op->isVar()) {
              VarData* vd = compiler->getVdById(op->getId());
              VarAttr* va;

              if (vd->getClass() == retClass) {
                // TODO: [COMPILER] Fix RetNode fetch.
                VI_MERGE_VAR(vd, va, 0, 0);
                va->setInRegs(i == 0 ? IntUtil::mask(kRegIndexAx) : IntUtil::mask(kRegIndexDx));
                va->addFlags(kVarAttrInReg);
                inRegs.add(retClass, va->getInRegs());
              }
            }
          }
          VI_END(node_);
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Call]
      // ----------------------------------------------------------------------

      case kNodeTypeCall: {
        X86X64CallNode* node = static_cast<X86X64CallNode*>(node_);
        X86X64FuncDecl* decl = node->getDecl();

        Operand* target = &node->_target;
        Operand* argList = node->_args;
        Operand* retList = node->_ret;

        func->addFuncFlags(kFuncFlagIsCaller);
        func->mergeCallStackSize(node->_x86Decl.getArgStackSize());
        node->_usedArgs = X86X64Context_getUsedArgs(this, node, decl);

        uint32_t i;
        uint32_t argCount = decl->getArgCount();
        uint32_t gpAllocableMask = gaRegs[kRegClassGp] & ~node->_usedArgs.get(kRegClassGp);

        VarData* vd;
        VarAttr* va;

        VI_BEGIN();

        // Function-call operand.
        if (target->isVar()) {
          vd = compiler->getVdById(target->getId());
          VI_MERGE_VAR(vd, va, 0, 0);

          va->addFlags(kVarAttrInReg | kVarAttrInCall);
          if (va->getInRegs() == 0)
            va->addAllocableRegs(gpAllocableMask);
        }
        else if (target->isMem()) {
          Mem* m = static_cast<Mem*>(target);

          if (OperandUtil::isVarId(m->getBase()) && m->isBaseIndexType()) {
            vd = compiler->getVdById(m->getBase());
            if (!vd->isStack()) {
              VI_MERGE_VAR(vd, va, 0, 0);
              if (m->getMemType() == kMemTypeBaseIndex) {
                va->addFlags(kVarAttrInReg | kVarAttrInCall);
                if (va->getInRegs() == 0)
                  va->addAllocableRegs(gpAllocableMask);
              }
              else {
                va->addFlags(kVarAttrInMem | kVarAttrInCall);
              }
            }
          }

          if (OperandUtil::isVarId(m->getIndex())) {
            // Restrict allocation to all registers except ESP/RSP/R12.
            vd = compiler->getVdById(m->getIndex());
            VI_MERGE_VAR(vd, va, 0, 0);

            va->addFlags(kVarAttrInReg | kVarAttrInCall);
            if ((va->getInRegs() & ~indexMask) == 0)
              va->andAllocableRegs(gpAllocableMask & indexMask);
          }
        }

        // Function-call arguments.
        for (i = 0; i < argCount; i++) {
          Operand* op = &argList[i];
          if (!op->isVar())
            continue;

          vd = compiler->getVdById(op->getId());
          VI_MERGE_VAR(vd, va, 0, 0);

          const FuncInOut& arg = decl->getArg(i);
          if (arg.hasRegIndex()) {
            uint32_t argType = arg.getVarType();
            uint32_t argClass = x86VarTypeToClass(argType);

            if (vd->getClass() == argClass) {
              va->addInRegs(IntUtil::mask(arg.getRegIndex()));
              va->addFlags(kVarAttrInReg | kVarAttrInArg);
            }
            else {
              va->addFlags(kVarAttrInConv | kVarAttrInArg);
            }
          }
          else {
            va->addArgStackCount();
            va->addFlags(kVarAttrInStack | kVarAttrInArg);
          }
        }

        // Function-call return(s).
        for (i = 0; i < 2; i++) {
          Operand* op = &retList[i];
          if (!op->isVar())
            continue;

          const FuncInOut& ret = decl->getRet(i);
          if (ret.hasRegIndex()) {
            uint32_t retType = ret.getVarType();
            uint32_t retClass = x86VarTypeToClass(retType);

            vd = compiler->getVdById(op->getId());
            VI_MERGE_VAR(vd, va, 0, 0);

            if (vd->getClass() == retClass) {
              va->setOutRegIndex(ret.getRegIndex());
              va->addFlags(kVarAttrOutReg | kVarAttrOutRet);
            }
            else {
              va->addFlags(kVarAttrOutConv | kVarAttrOutRet);
            }
          }
        }

        // Init clobbered.
        clobberedRegs.set(kRegClassGp, IntUtil::bits(_baseRegsCount) & (~decl->getPreserved(kRegClassGp)));
        clobberedRegs.set(kRegClassFp, IntUtil::bits(kRegCountFp   )                                     );
        clobberedRegs.set(kRegClassMm, IntUtil::bits(kRegCountMm   ) & (~decl->getPreserved(kRegClassMm)));
        clobberedRegs.set(kRegClassXy, IntUtil::bits(_baseRegsCount) & (~decl->getPreserved(kRegClassXy)));

        // Split all variables allocated in stack-only (i.e. if the variable is
        // only passed in stack; it doesn't matter how many times) and create
        // extra nodes having only stack moves. It improves x86 code, because
        // arguments can be moved on stack right after they are ready.
        for (i = 0; i < vaCount; i++) {
          VarAttr* va = &vaTmpList[i];

          if ((va->getFlags() & kVarAttrInAll) == (kVarAttrInArg | kVarAttrInStack)) {
            if (!X86X64Context_insertSArgNode(this, compiler, node, va->getVd(), gaRegs))
              goto _NoMemory;
            va->delFlags(kVarAttrInAll);
          }
        }

        VI_END(node_);
        break;
      }

      default:
        break;
    }

    node_ = next;
  } while (node_ != stop);

_Done:
  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Failure]
  // --------------------------------------------------------------------------

_NoMemory:
  return compiler->setError(kErrorNoHeapMemory);
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - AnalyzeFunc]
// ============================================================================

//! @internal
struct LivenessTarget {
  //! @brief Previous.
  LivenessTarget* prev;

  //! @brief Target node.
  TargetNode* node;
  //! @brief Jumped from.
  JumpNode* from;
};

Error X86X64Context::analyze() {
  FuncNode* func = getFunc();

  BaseNode* node = func->getEnd();
  JumpNode* from = NULL;

  uint32_t bLen = static_cast<uint32_t>(
    ((_contextVd.getLength() + VarBits::kEntityBits - 1) / VarBits::kEntityBits));

  LivenessTarget* ltCur = NULL;
  LivenessTarget* ltUnused = NULL;

  // No variables.
  if (bLen == 0)
    return kErrorOk;

  VarBits* bCur = newBits(bLen);
  if (bCur == NULL)
    goto _NoMemory;

  // Allocate bits for code visited first time.
_OnVisit:
  for (;;) {
    if (node->hasLiveness()) {
      if (bCur->_addBitsDelSource(node->getLiveness(), bCur, bLen))
        goto _OnPatch;
      else
        goto _OnDone;
    }

    VarBits* bTmp = copyBits(bCur, bLen);
    VarInst* vi = node->getVarInst<VarInst>();

    if (bTmp == NULL)
      goto _NoMemory;
    node->setLiveness(bTmp);

    if (vi != NULL) {
      uint32_t vaCount = vi->getVaCount();
      for (uint32_t i = 0; i < vaCount; i++) {
        VarAttr* va = vi->getVa(i);
        VarData* vd = va->getVd();

        uint32_t flags = va->getFlags();
        uint32_t ctxId = vd->getContextId();

        if ((flags & kVarAttrOutAll) && !(flags & kVarAttrInAll)) {
          // Write-Only.
          bTmp->setBit(ctxId);
          bCur->delBit(ctxId);
        }
        else {
          // Read-Only or Read/Write.
          bTmp->setBit(ctxId);
          bCur->setBit(ctxId);
        }
      }
    }

    if (node->getType() == kNodeTypeTarget)
      goto _OnTarget;

    if (node == func)
      goto _OnDone;
    node = node->getPrev();
  }

  // Patch already generated liveness bits.
_OnPatch:
  for (;;) {
    ASMJIT_ASSERT(node->hasLiveness());
    VarBits* bNode = node->getLiveness();

    if (!bNode->_addBitsDelSource(bCur, bLen))
      goto _OnDone;

    if (node->getType() == kNodeTypeTarget)
      goto _OnTarget;

    if (node == func)
      goto _OnDone;

    node = node->getPrev();
  }

_OnTarget:
  if (static_cast<TargetNode*>(node)->getNumRefs() != 0) {
    // Push a new LivenessTarget on the stack if needed.
    if (ltCur == NULL || ltCur->node != node) {
      LivenessTarget* ltTmp = ltUnused;

      if (ltTmp != NULL) {
        ltUnused = ltUnused->prev;
      }
      else {
        ltTmp = _zoneAllocator.allocT<LivenessTarget>(
          sizeof(LivenessTarget) - sizeof(VarBits) + bLen * sizeof(uintptr_t));

        if (ltTmp == NULL)
          goto _NoMemory;
      }

      ltTmp->prev = ltCur;
      ltTmp->node = static_cast<TargetNode*>(node);
      ltCur = ltTmp;

      from = static_cast<TargetNode*>(node)->getFrom();
      ASMJIT_ASSERT(from != NULL);
    }
    else {
      from = ltCur->from;
      goto _OnJumpNext;
    }

    // Visit/Patch.
    do {
      ltCur->from = from;
      bCur->copyBits(node->getLiveness(), bLen);

      if (!from->hasLiveness()) {
        node = from;
        goto _OnVisit;
      }

      if (bCur->delBits(from->getLiveness(), bLen)) {
        node = from;
        goto _OnPatch;
      }

_OnJumpNext:
      from = from->getJumpNext();
    } while (from != NULL);

    // Pop the current LivenessTarget from the stack.
    {
      LivenessTarget* ltTmp = ltCur;

      ltCur = ltCur->prev;
      ltTmp->prev = ltUnused;
      ltUnused = ltTmp;
    }
  }

  bCur->copyBits(node->getLiveness(), bLen);
  node = node->getPrev();

  if (node->isJmp() || !node->isFetched())
    goto _OnDone;

  if (!node->hasLiveness())
    goto _OnVisit;

  if (bCur->delBits(node->getLiveness(), bLen))
    goto _OnPatch;

_OnDone:
  if (ltCur != NULL) {
    node = ltCur->node;
    from = ltCur->from;

    goto _OnJumpNext;
  }
  return kErrorOk;

_NoMemory:
  return setError(kErrorNoHeapMemory);
}

// ============================================================================
// [asmjit::x86x64::X86X64BaseAlloc]
// ============================================================================

struct X86X64BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86X64BaseAlloc(X86X64Context* context) {
    _context = context;
    _compiler = context->getCompiler();
  }
  ASMJIT_INLINE ~X86X64BaseAlloc() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get the context.
  ASMJIT_INLINE X86X64Context* getContext() const { return _context; }
  //! @brief Get the current state (always the same instance as X86X64Context::_x86State).
  ASMJIT_INLINE VarState* getState() const { return _context->getState(); }

  //! @brief Get the node.
  ASMJIT_INLINE BaseNode* getNode() const { return _node; }

  //! @brief Get VarAttr list (all).
  ASMJIT_INLINE VarAttr* getVaList() const { return _vaList[0]; }
  //! @brief Get VarAttr list (per class).
  ASMJIT_INLINE VarAttr* getVaListByClass(uint32_t c) const { return _vaList[c]; }

  //! @brief Get VarAttr count (all).
  ASMJIT_INLINE uint32_t getVaCount() const { return _vaCount; }
  //! @brief Get VarAttr count (per class).
  ASMJIT_INLINE uint32_t getVaCountByClass(uint32_t c) const { return _count.get(c); }

  //! @brief Get whether all variables of class @a c are done.
  ASMJIT_INLINE bool isVaDone(uint32_t c) const { return _done.get(c) == _count.get(c); }

  //! @brief Get how many variables have been allocated.
  ASMJIT_INLINE uint32_t getVaDone(uint32_t c) const { return _done.get(c); }

  ASMJIT_INLINE void addVaDone(uint32_t c, uint32_t n = 1) { _done.add(c, n); }

  //! @brief Get number of allocable registers per class.
  ASMJIT_INLINE uint32_t getGaRegs(uint32_t c) const {
    return _context->_gaRegs[c];
  }

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods by X86X64Context::translate().

  ASMJIT_INLINE void init(BaseNode* node, VarInst* vi);
  ASMJIT_INLINE void cleanup();

  // --------------------------------------------------------------------------
  // [Unuse]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void unuseBefore();

  template<int C>
  ASMJIT_INLINE void unuseAfter();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Context.
  X86X64Context* _context;
  //! @brief Compiler.
  X86X64Compiler* _compiler;

  //! @brief Node.
  BaseNode* _node;

  //! @brief Variable instructions.
  VarInst* _vi;
  //! @brief VarAttr list (per register class).
  VarAttr* _vaList[4];

  //! @brief Count of all VarAttr's.
  uint32_t _vaCount;

  //! @brief VarAttr's total counter.
  RegCount _count;
  //! @brief VarAttr's done counter.
  RegCount _done;
};

// ============================================================================
// [asmjit::x86x64::X86X64BaseAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86X64BaseAlloc::init(BaseNode* node, VarInst* vi) {
  _node = node;
  _vi = vi;

  // We have to set the correct cursor in case any instruction is emitted
  // during the allocation phase; it has to be emitted before the current
  // instruction.
  _compiler->_setCursor(node->getPrev());

  // Setup the lists of variables.
  {
    VarAttr* va = vi->getVaList();
    _vaList[kRegClassGp] = va;
    _vaList[kRegClassFp] = va + vi->getVaStart(kRegClassFp);
    _vaList[kRegClassMm] = va + vi->getVaStart(kRegClassMm);
    _vaList[kRegClassXy] = va + vi->getVaStart(kRegClassXy);
  }

  // Setup counters.
  _vaCount = vi->getVaCount();

  _count = vi->_count;
  _done.reset();

  // Connect Vd->Va.
  for (uint32_t i = 0; i < _vaCount; i++) {
    VarAttr* va = &_vaList[0][i];
    VarData* vd = va->getVd();

    vd->setVa(va);
  }
}

ASMJIT_INLINE void X86X64BaseAlloc::cleanup() {
  // Disconnect Vd->Va.
  for (uint32_t i = 0; i < _vaCount; i++) {
    VarAttr* va = &_vaList[0][i];
    VarData* vd = va->getVd();

    vd->setVa(NULL);
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64BaseAlloc - Unuse]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64BaseAlloc::unuseBefore() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  const uint32_t checkFlags =
    kVarAttrInOutReg |
    kVarAttrInMem    |
    kVarAttrInArg    |
    kVarAttrInStack  |
    kVarAttrInCall   |
    kVarAttrInConv   ;

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if ((va->getFlags() & checkFlags) == kVarAttrOutReg) {
      _context->unuse<C>(va->getVd());
    }
  }
}

template<int C>
ASMJIT_INLINE void X86X64BaseAlloc::unuseAfter() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if (va->getFlags() & kVarAttrUnuse)
      _context->unuse<C>(va->getVd());
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64VarAlloc]
// ============================================================================

//! @internal
//!
//! @brief Register allocator context (asm instructions).
struct X86X64VarAlloc : public X86X64BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86X64VarAlloc(X86X64Context* context) : X86X64BaseAlloc(context) {}
  ASMJIT_INLINE ~X86X64VarAlloc() {}

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error run(BaseNode* node);

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods by X86X64Context::translate().

  ASMJIT_INLINE void init(BaseNode* node, VarInst* vi);
  ASMJIT_INLINE void cleanup();

  // --------------------------------------------------------------------------
  // [Plan / Spill / Alloc]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void plan();

  template<int C>
  ASMJIT_INLINE void spill();

  template<int C>
  ASMJIT_INLINE void alloc();

  // --------------------------------------------------------------------------
  // [GuessAlloc / GuessSpill]
  // --------------------------------------------------------------------------

  //! @brief Guess which register is the best candidate for 'vd' from
  //! 'allocableRegs'.
  //!
  //! The guess is based on looking ahead and inspecting register allocator
  //! instructions. The main reason is to prevent allocation to a register
  //! which is needed by next instruction(s). The guess look tries to go as far
  //! as possible, after the remaining registers are zero, the mask of previous
  //! registers (called 'safeRegs') is returned.
  template<int C>
  ASMJIT_INLINE uint32_t guessAlloc(VarData* vd, uint32_t allocableRegs);

  //! @brief Guess whether to move the given 'vd' instead of spill.
  template<int C>
  ASMJIT_INLINE uint32_t guessSpill(VarData* vd, uint32_t allocableRegs);

  // --------------------------------------------------------------------------
  // [Modified]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void modified();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Will alloc to these registers.
  RegMask _willAlloc;
  //! @brief Will spill these registers.
  RegMask _willSpill;
};

// ============================================================================
// [asmjit::X86X64VarAlloc - Run]
// ============================================================================

ASMJIT_INLINE Error X86X64VarAlloc::run(BaseNode* node_) {
  // Initialize.
  VarInst* vi = node_->getVarInst<VarInst>();
  if (vi == NULL)
    return kErrorOk;

  // Initialize the allocator; connect Vd->Va.
  init(node_, vi);

  // Unuse overwritten variables.
  unuseBefore<kRegClassGp>();
  unuseBefore<kRegClassMm>();
  unuseBefore<kRegClassXy>();

  // Plan the allocation. Planner assigns input/output registers for each
  // variable and decides whether to allocate it in register or stack.
  plan<kRegClassGp>();
  plan<kRegClassMm>();
  plan<kRegClassXy>();

  // Spill all variables marked by plan().
  spill<kRegClassGp>();
  spill<kRegClassMm>();
  spill<kRegClassXy>();

  // Alloc all variables marked by plan().
  alloc<kRegClassGp>();
  alloc<kRegClassMm>();
  alloc<kRegClassXy>();

  // Translate node operands.
  if (node_->getType() == kNodeTypeInst) {
    InstNode* node = static_cast<InstNode*>(node_);
    ASMJIT_PROPAGATE_ERROR(X86X64Context_translateOperands(_context, node->getOpList(), node->getOpCount()));
  }
  else if (node_->getType() == kNodeTypeSArg) {
    SArgNode* node = static_cast<SArgNode*>(node_);
    VarData* vd = node->getVd();

    X86X64CallNode* call = static_cast<X86X64CallNode*>(node->getCall());
    X86X64FuncDecl* decl = call->getDecl();

    uint32_t argCount = decl->getArgCount();
    for (uint32_t i = 0; i < argCount; i++) {
      Operand& op = call->getArg(i);
      FuncInOut& arg = decl->getArg(i);

      if (!op.isVar() || op.getId() != vd->getId())
        continue;

      Mem dst = ptr(_context->_zsp, -static_cast<int>(_context->getRegSize()) + arg.getStackOffset());
      _context->emitMoveVarOnStack(arg.getVarType(), &dst, vd->getType(), vd->getRegIndex());
    }
  }

  // Mark variables as modified.
  modified<kRegClassGp>();
  modified<kRegClassMm>();
  modified<kRegClassXy>();

  // Cleanup; disconnect Vd->Va.
  cleanup();

  // Update clobbered mask.
  _context->_clobberedRegs.add(_willAlloc);
  _context->_clobberedRegs.add(vi->_clobberedRegs);

  // Unuse.
  unuseAfter<kRegClassGp>();
  unuseAfter<kRegClassMm>();
  unuseAfter<kRegClassXy>();

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86x64::X86X64VarAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86X64VarAlloc::init(BaseNode* node, VarInst* vi) {
  X86X64BaseAlloc::init(node, vi);

  // These will block planner from assigning them during planning. Planner will
  // add more registers when assigning registers to variables that don't need
  // any specific register.
  _willAlloc = vi->_inRegs;
  _willAlloc.add(vi->_outRegs);
  _willSpill.reset();
}

ASMJIT_INLINE void X86X64VarAlloc::cleanup() {
  X86X64BaseAlloc::cleanup();
}

// ============================================================================
// [asmjit::x86x64::X86X64VarAlloc - Plan / Spill / Alloc]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64VarAlloc::plan() {
  if (isVaDone(C))
    return;

  uint32_t i;

  uint32_t willAlloc = _willAlloc.get(C);
  uint32_t willFree = 0;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Calculate 'willAlloc' and 'willFree' masks based on mandatory masks.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();
    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = (regIndex != kInvalidReg) ? IntUtil::mask(regIndex) : 0;

    if ((vaFlags & kVarAttrInOutReg) != 0) {
      // Planning register allocation. First check whether the variable is
      // already allocated in register and if it can stay allocated there.
      //
      // The following conditions may happen:
      //
      // a) Allocated register is one of the mandatoryRegs.
      // b) Allocated register is one of the allocableRegs.
      uint32_t mandatoryRegs = va->getInRegs();
      uint32_t allocableRegs = va->getAllocableRegs();

      if (regMask != 0) {
        // Special path for planning output-only registers.
        if ((vaFlags & kVarAttrInOutReg) == kVarAttrOutReg) {
          uint32_t outRegIndex = va->getOutRegIndex();
          mandatoryRegs = (outRegIndex != kInvalidReg) ? IntUtil::mask(outRegIndex) : 0;

          if ((mandatoryRegs | allocableRegs) & regMask) {
            va->setOutRegIndex(regIndex);
            va->addFlags(kVarAttrAllocOutDone);

            if (mandatoryRegs & regMask) {
              // Case 'a' - 'willAlloc' contains initially all inRegs from all VarAttr's.
              ASMJIT_ASSERT((willAlloc & regMask) != 0);
            }
            else {
              // Case 'b'.
              va->setOutRegIndex(regIndex);
              willAlloc |= regMask;
            }

            addVaDone(C);
            continue;
          }
        }
        else {
          if ((mandatoryRegs | allocableRegs) & regMask) {
            va->setInRegIndex(regIndex);
            va->addFlags(kVarAttrAllocInDone);

            if (mandatoryRegs & regMask) {
              // Case 'a' - 'willAlloc' contains initially all inRegs from all VarAttr's.
              ASMJIT_ASSERT((willAlloc & regMask) != 0);
            }
            else {
              // Case 'b'.
              va->addInRegs(regMask);
              willAlloc |= regMask;
            }

            addVaDone(C);
            continue;
          }
        }
      }

      // Variable is not allocated or allocated in register that doesn't
      // match inRegs or allocableRegs. The next step is to pick the best
      // register for this variable. If inRegs contains any register the
      // decision is simple - we have to follow, in other case will use
      // the advantage of guessAlloc() to find a register (or registers)
      // by looking ahead. But the best way to find a good register is not
      // here since now we have no information about the registers that
      // will be freed. So instead of finding register here, we just mark
      // the current register (if variable is allocated) as 'willFree' so
      // the planner can use this information in second step to plan other
      // allocation of other variables.
      willFree |= regMask;
      continue;
    }
    else {
      // Memory access - if variable is allocated it has to be freed.
      if (regMask != 0) {
        willFree |= regMask;
        continue;
      }
      else {
        va->addFlags(kVarAttrAllocInDone);
        addVaDone(C);
        continue;
      }
    }
  }

  // Occupied registers without 'willFree' registers; contains basically
  // all the registers we can use to allocate variables without inRegs
  // speficied.
  uint32_t occupied = state->_occupied.get(C) & ~willFree;
  uint32_t willSpill = 0;

  // Find the best registers for variables that are not allocated yet.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();

    if ((vaFlags & kVarAttrInOutReg) != 0) {
      if ((vaFlags & kVarAttrInOutReg) == kVarAttrOutReg) {
        if (vaFlags & kVarAttrAllocOutDone)
          continue;

        // We skip all registers that have assigned outRegIndex. The only
        // important thing is to not forget to spill it if occupied.
        if (va->hasOutRegIndex()) {
          uint32_t outRegs = IntUtil::mask(va->getOutRegIndex());
          willSpill |= occupied & outRegs;
          continue;
        }
      }
      else {
        if (vaFlags & kVarAttrAllocInDone)
          continue;

        // We skip all registers that have assigned inRegIndex (it indicates that
        // the register to allocate into is known).
        if (va->hasInRegIndex()) {
          uint32_t inRegs = va->getInRegs();
          willSpill |= occupied & inRegs;
          continue;
        }
      }

      uint32_t m = va->getInRegs();
      if (va->hasOutRegIndex())
        m |= IntUtil::mask(va->getOutRegIndex());

      m = va->getAllocableRegs() & ~(willAlloc ^ m);
      m = guessAlloc<C>(vd, m);
      ASMJIT_ASSERT(m != 0);

      uint32_t candidateRegs = m & ~occupied;
      uint32_t regIndex;
      uint32_t regMask;

      if (candidateRegs == 0) {
        candidateRegs = m & occupied & ~state->_modified.get(C);
        if (candidateRegs == 0)
          candidateRegs = m;
      }

      regIndex = IntUtil::findFirstBit(candidateRegs);
      regMask = IntUtil::mask(regIndex);

      if ((vaFlags & kVarAttrInOutReg) == kVarAttrOutReg) {
        va->setOutRegIndex(regIndex);
      }
      else {
        va->setInRegIndex(regIndex);
        va->setInRegs(regMask);
      }

      willAlloc |= regMask;
      willSpill |= regMask & occupied;
      willFree &= ~regMask;
      occupied |= regMask;
      continue;
    }
  }

  // Set calculated masks back to the allocator; needed by spill() and alloc().
  _willSpill.set(C, willSpill);
  _willAlloc.set(C, willAlloc);
}

template<int C>
ASMJIT_INLINE void X86X64VarAlloc::spill() {
  uint32_t m = _willSpill.get(C);
  uint32_t i = static_cast<uint32_t>(0) - 1;

  if (m == 0)
    return;

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Available registers for decision if move has any benefit over spill.
  uint32_t availableRegs = getGaRegs(C) & ~(state->_occupied.get(C) | m | _willAlloc.get(C));

  do {
    // We always advance one more to destroy the bit that we have found.
    uint32_t bitIndex = IntUtil::findFirstBit(m) + 1;

    i += bitIndex;
    m >>= bitIndex;

    VarData* vd = sVars[i];
    ASMJIT_ASSERT(vd != NULL);
    ASMJIT_ASSERT(vd->getVa() == NULL);

    if (vd->isModified() && availableRegs) {
      uint32_t m = guessSpill<C>(vd, availableRegs);

      if (m != 0) {
        uint32_t regIndex = IntUtil::findFirstBit(m);
        uint32_t regMask = IntUtil::mask(regIndex);

        _context->move<C>(vd, regIndex);
        availableRegs ^= regMask;
        continue;
      }
    }

    _context->spill<C>(vd);
  } while (m != 0);
}

template<int C>
ASMJIT_INLINE void X86X64VarAlloc::alloc() {
  if (isVaDone(C))
    return;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  bool didWork;

  // Alloc 'in' regs.
  do {
    didWork = false;
    for (i = 0; i < count; i++) {
      VarAttr* aVa = &list[i];
      VarData* aVd = aVa->getVd();

      if ((aVa->getFlags() & (kVarAttrInReg | kVarAttrAllocInDone)) != kVarAttrInReg)
        continue;

      uint32_t aIndex = aVd->getRegIndex();
      uint32_t bIndex = aVa->getInRegIndex();

      // Shouldn't be the same.
      ASMJIT_ASSERT(aIndex != bIndex);

      VarData* bVd = getState()->getListByClass(C)[bIndex];
      if (bVd != NULL) {
        // Gp registers only - Swap two registers if we can solve two
        // allocation tasks by a single 'xchg' instruction, swapping
        // two registers required by the instruction/node or one register
        // required with another non-required.
        if (C == kRegClassGp && aIndex != kInvalidReg) {
          VarAttr* bVa = bVd->getVa();
          _context->swapGp(aVd, bVd);

          aVa->addFlags(kVarAttrAllocInDone);
          addVaDone(C);

          // Doublehit, two registers allocated by a single swap.
          if (bVa != NULL && bVa->getInRegIndex() == aIndex) {
            bVa->addFlags(kVarAttrAllocInDone);
            addVaDone(C);
          }

          didWork = true;
          continue;
        }
      }
      else if (aIndex != kInvalidReg) {
        _context->move<C>(aVd, bIndex);

        aVa->addFlags(kVarAttrAllocInDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
      else {
        _context->alloc<C>(aVd, bIndex);

        aVa->addFlags(kVarAttrAllocInDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
    }
  } while (didWork);

  // Alloc 'out' regs.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    if ((va->getFlags() & (kVarAttrInOutReg | kVarAttrAllocOutDone)) != kVarAttrOutReg)
      continue;

    uint32_t regIndex = va->getOutRegIndex();
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    if (vd->getRegIndex() != regIndex) {
      ASMJIT_ASSERT(sVars[regIndex] == NULL);
      _context->attach<C>(vd, regIndex, false);
    }

    va->addFlags(kVarAttrAllocOutDone);
    addVaDone(C);
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64VarAlloc - GuessAlloc / GuessSpill]
// ============================================================================

template<int C>
ASMJIT_INLINE uint32_t X86X64VarAlloc::guessAlloc(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  // Stop now if there is only one bit (register) set in 'allocableRegs' mask.
  if (IntUtil::isPowerOf2(allocableRegs))
    return allocableRegs;

  uint32_t i;
  uint32_t safeRegs = allocableRegs;
  uint32_t maxLookAhead = _compiler->getMaxLookAhead();

  // Look ahead and calculate mask of special registers on both - input/output.
  BaseNode* node = _node;
  for (i = 0; i < maxLookAhead; i++) {
    // Stop on 'RetNode' and 'EndNode.
    if (node->hasFlag(kNodeFlagIsRet))
      break;

    // Stop on conditional jump, we don't follow them.
    if (node->hasFlag(kNodeFlagIsJcc))
      break;

    // Advance on non-conditional jump.
    if (node->hasFlag(kNodeFlagIsJmp))
      node = static_cast<JumpNode*>(node)->getTarget();

    node = node->getNext();
    ASMJIT_ASSERT(node != NULL);

    VarInst* vi = node->getVarInst<VarInst>();
    if (vi != NULL) {
      VarAttr* va = vi->findVaByClass(C, vd);
      if (va != NULL) {
        uint32_t inRegs = va->getInRegs();
        if (inRegs != 0) {
          safeRegs = allocableRegs;
          allocableRegs &= inRegs;

          if (allocableRegs == 0)
            goto _UseSafeRegs;
          else
            return allocableRegs;
        }
      }

      safeRegs = allocableRegs;
      allocableRegs &= ~(vi->_inRegs.get(C) | vi->_outRegs.get(C) | vi->_clobberedRegs.get(C));

      if (allocableRegs == 0)
        break;
    }
  }

_UseSafeRegs:
  return safeRegs;
}

template<int C>
ASMJIT_INLINE uint32_t X86X64VarAlloc::guessSpill(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  return 0;
}

// ============================================================================
// [asmjit::x86x64::X86X64VarAlloc - Modified]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64VarAlloc::modified() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if (va->hasFlag(kVarAttrOutReg)) {
      VarData* vd = va->getVd();

      uint32_t regIndex = vd->getRegIndex();
      uint32_t regMask = IntUtil::mask(regIndex);

      vd->setModified(true);
      _context->_x86State._modified.add(C, regMask);
    }
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc]
// ============================================================================

//! @internal
//!
//! @brief Register allocator context (function call).
struct X86X64CallAlloc : public X86X64BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86X64CallAlloc(X86X64Context* context) : X86X64BaseAlloc(context) {}
  ASMJIT_INLINE ~X86X64CallAlloc() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get the node.
  ASMJIT_INLINE X86X64CallNode* getNode() const { return static_cast<X86X64CallNode*>(_node); }

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error run(X86X64CallNode* node);

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods from X86X64Context::translate().

  ASMJIT_INLINE void init(X86X64CallNode* node, VarInst* vi);
  ASMJIT_INLINE void cleanup();

  // --------------------------------------------------------------------------
  // [Plan / Alloc / Spill / Move]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void plan();

  template<int C>
  ASMJIT_INLINE void spill();

  template<int C>
  ASMJIT_INLINE void alloc();

  // --------------------------------------------------------------------------
  // [AllocVars/Imms]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void allocVarsOnStack();
  ASMJIT_INLINE void allocImmsOnStack();

  // --------------------------------------------------------------------------
  // [GuessAlloc / GuessSpill]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE uint32_t guessAlloc(VarData* vd, uint32_t allocableRegs);

  template<int C>
  ASMJIT_INLINE uint32_t guessSpill(VarData* vd, uint32_t allocableRegs);

  // --------------------------------------------------------------------------
  // [Save]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void save();

  // --------------------------------------------------------------------------
  // [Clobber]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void clobber();

  // --------------------------------------------------------------------------
  // [Ret]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void ret();

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void stackArgDone(uint32_t argMask) {
    ASMJIT_ASSERT(_stackArgsMask & argMask);
    _stackArgsMask ^= argMask;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Will alloc to these registers.
  RegMask _willAlloc;
  //! @brief Will spill these registers.
  RegMask _willSpill;

  //! @brief Pending stack-arguments mask.
  uint32_t _stackArgsMask;

  //! @brief Argument index to VarAttr mapping.
  VarAttr* _argToVa[kFuncArgCountLoHi];
};

// ============================================================================
// [asmjit::X86X64CallAlloc - Run]
// ============================================================================

ASMJIT_INLINE Error X86X64CallAlloc::run(X86X64CallNode* node) {
  // Initialize.
  VarInst* vi = node->getVarInst<VarInst>();
  if (vi == NULL)
    return kErrorOk;

  // Initialize the allocator; prepare basics and connect Vd->Va.
  init(node, vi);

  // Move whatever can be moved on the stack.
  allocVarsOnStack();

  // Plan register allocation. Planner is only able to assign one register per
  // variable. If any variable is used multiple times it will be handled later.
  plan<kRegClassGp>();
  plan<kRegClassMm>();
  plan<kRegClassXy>();

  // Spill.
  spill<kRegClassGp>();
  spill<kRegClassMm>();
  spill<kRegClassXy>();

  // Alloc.
  alloc<kRegClassGp>();
  alloc<kRegClassMm>();
  alloc<kRegClassXy>();

  // Move the remaining variables on the stack.
  allocVarsOnStack();

  // Unuse clobbered registers that are not used to pass function arguments and
  // save variables used to pass function arguments that will be reused later on.
  save<kRegClassGp>();
  save<kRegClassMm>();
  save<kRegClassXy>();

  // Allocate immediates in registers and on the stack.
  allocImmsOnStack();

  // Duplicate/Convert.
  // TODO:

  // Translate call operand.
  ASMJIT_PROPAGATE_ERROR(X86X64Context_translateOperands(_context, &node->_target, 1));

  // Clobber.
  clobber<kRegClassGp>();
  clobber<kRegClassMm>();
  clobber<kRegClassXy>();

  // If any instruction has to be emitted to properly handle function return it
  // has to be emitted right after the call, thus the cursor has to be changed.
  _compiler->_setCursor(node);

  // Return.
  ret();

  // Unuse.
  unuseAfter<kRegClassGp>();
  unuseAfter<kRegClassMm>();
  unuseAfter<kRegClassXy>();

  // Cleanup; disconnect Vd->Va.
  cleanup();

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86X64CallAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86X64CallAlloc::init(X86X64CallNode* node, VarInst* vi) {
  X86X64BaseAlloc::init(node, vi);

  // Create mask of all registers that will be used to pass function arguments.
  _willAlloc = node->_usedArgs;
  _willSpill.reset();

  // Initialize argToVa[] array and pending stack-args mask/count.
  X86X64FuncDecl* decl = node->getDecl();
  uint32_t i, mask;

  Operand* argList = node->_args;
  uint32_t argCount = decl->getArgCount();

  _stackArgsMask = 0;
  ::memset(_argToVa, 0, kFuncArgCountLoHi * sizeof(VarAttr*));

  for (i = 0, mask = 1; i < argCount; i++, mask <<= 1) {
    Operand* op = &argList[i];

    if (!op->isVar())
      continue;

    VarData* vd = _compiler->getVdById(op->getId());
    VarAttr* va = vd->getVa();
    _argToVa[i] = va;

    const FuncInOut& arg = decl->getArg(i);
    if (!arg.hasStackOffset())
      continue;

    if ((va->getFlags() & kVarAttrInAll) == 0)
      continue;
    _stackArgsMask |= mask;
  }
}

ASMJIT_INLINE void X86X64CallAlloc::cleanup() {
  X86X64BaseAlloc::cleanup();
}

// ============================================================================
// [asmjit::X86X64CallAlloc - Plan / Spill / Alloc]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64CallAlloc::plan() {
  uint32_t i;
  uint32_t clobbered = _vi->_clobberedRegs.get(C);

  uint32_t willAlloc = _willAlloc.get(C);
  uint32_t willFree = clobbered & ~willAlloc;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Calculate 'willAlloc' and 'willFree' masks based on mandatory masks.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();
    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = (regIndex != kInvalidReg) ? IntUtil::mask(regIndex) : 0;

    if ((vaFlags & kVarAttrInReg) != 0) {
      // Planning register allocation. First check whether the variable is
      // already allocated in register and if it can stay there. Function
      // arguments are passed either in a specific register or in stack so
      // we care mostly of mandatory registers.
      uint32_t inRegs = va->getInRegs();

      if (inRegs == 0) {
        inRegs = va->getAllocableRegs();
      }

      if (regMask & inRegs) {
        va->setInRegIndex(regIndex);
        va->addFlags(kVarAttrAllocInDone);
        addVaDone(C);
      }
      else {
        willFree |= regMask;
      }
    }
    else {
      // Memory access - if variable is allocated it has to be freed.
      if (regMask != 0) {
        willFree |= regMask;
      }
      else {
        va->addFlags(kVarAttrAllocInDone);
        addVaDone(C);
      }
    }
  }

  // Occupied registers without 'willFree' registers; contains basically
  // all the registers we can use to allocate variables without inRegs
  // speficied.
  uint32_t occupied = state->_occupied.get(C) & ~willFree;
  uint32_t willSpill = 0;

  // Find the best registers for variables that are not allocated yet. Only
  // useful for Gp registers used as call operand.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();
    if ((vaFlags & kVarAttrAllocInDone) != 0 || (vaFlags & kVarAttrInReg) == 0)
      continue;

    // All registers except Gp used by call itself must have inRegIndex.
    uint32_t m = va->getInRegs();
    if (C != kRegClassGp || m) {
      ASMJIT_ASSERT(m != 0);
      va->setInRegIndex(IntUtil::findFirstBit(m));
      willSpill |= occupied & m;
      continue;
    }

    m = va->getAllocableRegs() & ~(willAlloc ^ m);
    m = guessAlloc<C>(vd, m);
    ASMJIT_ASSERT(m != 0);

    uint32_t candidateRegs = m & ~occupied;
    if (candidateRegs == 0) {
      candidateRegs = m & occupied & ~state->_modified.get(C);
      if (candidateRegs == 0)
        candidateRegs = m;
    }

    if (!(vaFlags & (kVarAttrOutReg | kVarAttrUnuse)) && (candidateRegs & ~clobbered))
      candidateRegs &= ~clobbered;

    uint32_t regIndex = IntUtil::findFirstBit(candidateRegs);
    uint32_t regMask = IntUtil::mask(regIndex);

    va->setInRegIndex(regIndex);
    va->setInRegs(regMask);

    willAlloc |= regMask;
    willSpill |= regMask & occupied;
    willFree &= ~regMask;

    occupied |= regMask;
    continue;
  }

  // Set calculated masks back to the allocator; needed by spill() and alloc().
  _willSpill.set(C, willSpill);
  _willAlloc.set(C, willAlloc);
}

template<int C>
ASMJIT_INLINE void X86X64CallAlloc::spill() {
  uint32_t m = _willSpill.get(C);
  uint32_t i = static_cast<uint32_t>(0) - 1;

  if (m == 0)
    return;

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Available registers for decision if move has any benefit over spill.
  uint32_t availableRegs = getGaRegs(C) & ~(state->_occupied.get(C) | m | _willAlloc.get(C));

  do {
    // We always advance one more to destroy the bit that we have found.
    uint32_t bitIndex = IntUtil::findFirstBit(m) + 1;

    i += bitIndex;
    m >>= bitIndex;

    VarData* vd = sVars[i];
    ASMJIT_ASSERT(vd != NULL);
    ASMJIT_ASSERT(vd->getVa() == NULL);

    if (vd->isModified() && availableRegs) {
      uint32_t m = guessSpill<C>(vd, availableRegs);

      if (m != 0) {
        uint32_t regIndex = IntUtil::findFirstBit(m);
        uint32_t regMask = IntUtil::mask(regIndex);

        _context->move<C>(vd, regIndex);
        availableRegs ^= regMask;
        continue;
      }
    }

    _context->spill<C>(vd);
  } while (m != 0);
}

template<int C>
ASMJIT_INLINE void X86X64CallAlloc::alloc() {
  if (isVaDone(C))
    return;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  bool didWork;

  do {
    didWork = false;
    for (i = 0; i < count; i++) {
      VarAttr* aVa = &list[i];
      VarData* aVd = aVa->getVd();

      if ((aVa->getFlags() & (kVarAttrInReg | kVarAttrAllocInDone)) != kVarAttrInReg)
        continue;

      uint32_t aIndex = aVd->getRegIndex();
      uint32_t bIndex = aVa->getInRegIndex();

      // Shouldn't be the same.
      ASMJIT_ASSERT(aIndex != bIndex);

      VarData* bVd = getState()->getListByClass(C)[bIndex];
      if (bVd != NULL) {
        VarAttr* bVa = bVd->getVa();

        // Gp registers only - Swap two registers if we can solve two
        // allocation tasks by a single 'xchg' instruction, swapping
        // two registers required by the instruction/node or one register
        // required with another non-required.
        if (C == kRegClassGp) {
          _context->swapGp(aVd, bVd);

          aVa->addFlags(kVarAttrAllocInDone);
          addVaDone(C);

          // Doublehit, two registers allocated by a single swap.
          if (bVa != NULL && bVa->getInRegIndex() == aIndex) {
            bVa->addFlags(kVarAttrAllocInDone);
            addVaDone(C);
          }

          didWork = true;
          continue;
        }
      }
      else if (aIndex != kInvalidReg) {
        _context->move<C>(aVd, bIndex);

        aVa->addFlags(kVarAttrAllocInDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
      else {
        _context->alloc<C>(aVd, bIndex);

        aVa->addFlags(kVarAttrAllocInDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
    }
  } while (didWork);
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc - AllocVars/Imms]
// ============================================================================

ASMJIT_INLINE void X86X64CallAlloc::allocVarsOnStack() {
  if (_stackArgsMask == 0)
    return;

  X86X64CallNode* node = getNode();
  X86X64FuncDecl* decl = node->getDecl();

  uint32_t i;
  uint32_t mask;

  uint32_t argCount = decl->getArgCount();
  Operand* argList = node->_args;

  for (i = 0, mask = 1; i < argCount; i++, mask <<= 1) {
    if ((_stackArgsMask & mask) == 0)
      continue;

    VarAttr* va = _argToVa[i];
    ASMJIT_ASSERT(va != NULL);
    ASMJIT_ASSERT(va->getArgStackCount() != 0);

    VarData* vd = va->getVd();
    uint32_t regIndex = vd->getRegIndex();

    if (regIndex == kInvalidReg)
      continue;

    const FuncInOut& arg = decl->getArg(i);
    Mem dst = ptr(_context->_zsp, -static_cast<int>(_context->getRegSize()) + arg.getStackOffset());

    _context->emitMoveVarOnStack(arg.getVarType(), &dst, vd->getType(), regIndex);
    stackArgDone(mask);
  }
}

ASMJIT_INLINE void X86X64CallAlloc::allocImmsOnStack() {
  X86X64CallNode* node = getNode();
  X86X64FuncDecl* decl = node->getDecl();

  uint32_t argCount = decl->getArgCount();
  Operand* argList = node->_args;

  for (uint32_t i = 0; i < argCount; i++) {
    VarAttr* va = _argToVa[i];
    if (va != NULL)
      continue;

    const Imm& imm = static_cast<const Imm&>(node->getArg(i));
    const FuncInOut& arg = decl->getArg(i);

    if (arg.hasStackOffset()) {
      Mem dst = ptr(_context->_zsp, -static_cast<int>(_context->getRegSize()) + arg.getStackOffset());
      _context->emitMoveImmOnStack(arg.getVarType(), &dst, &imm);
    }
    else {
      _context->emitMoveImmToReg(arg.getVarType(), arg.getRegIndex(), &imm);
    }
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc - GuessAlloc / GuessSpill]
// ============================================================================

template<int C>
ASMJIT_INLINE uint32_t X86X64CallAlloc::guessAlloc(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  // Stop now if there is only one bit (register) set in 'allocableRegs' mask.
  if (IntUtil::isPowerOf2(allocableRegs))
    return allocableRegs;

  uint32_t i;
  uint32_t safeRegs = allocableRegs;
  uint32_t maxLookAhead = _compiler->getMaxLookAhead();

  // Look ahead and calculate mask of special registers on both - input/output.
  BaseNode* node = _node;
  for (i = 0; i < maxLookAhead; i++) {
    // Stop on 'RetNode' and 'EndNode.
    if (node->hasFlag(kNodeFlagIsRet))
      break;

    // Stop on conditional jump, we don't follow them.
    if (node->hasFlag(kNodeFlagIsJcc))
      break;

    // Advance on non-conditional jump.
    if (node->hasFlag(kNodeFlagIsJmp))
      node = static_cast<JumpNode*>(node)->getTarget();

    node = node->getNext();
    ASMJIT_ASSERT(node != NULL);

    VarInst* vi = node->getVarInst<VarInst>();
    if (vi != NULL) {
      VarAttr* va = vi->findVaByClass(C, vd);
      if (va != NULL) {
        uint32_t inRegs = va->getInRegs();
        if (inRegs != 0) {
          safeRegs = allocableRegs;
          allocableRegs &= inRegs;

          if (allocableRegs == 0)
            goto _UseSafeRegs;
          else
            return allocableRegs;
        }
      }

      safeRegs = allocableRegs;
      allocableRegs &= ~(vi->_inRegs.get(C) | vi->_outRegs.get(C) | vi->_clobberedRegs.get(C));

      if (allocableRegs == 0)
        break;
    }
  }

_UseSafeRegs:
  return safeRegs;
}

template<int C>
ASMJIT_INLINE uint32_t X86X64CallAlloc::guessSpill(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  return 0;
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc - Save]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64CallAlloc::save() {
  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  uint32_t affected = _vi->_clobberedRegs.get(C) & state->_occupied.get(C) & state->_modified.get(C);

  for (i = 0; affected != 0; i++, affected >>= 1) {
    if (affected & 0x1) {
      VarData* vd = sVars[i];
      ASMJIT_ASSERT(vd != NULL);
      ASMJIT_ASSERT(vd->isModified());

      VarAttr* va = vd->getVa();
      if (va == NULL || !(va->getFlags() & kVarAttrInAll)) {
        _context->save<C>(vd);
      }
    }
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc - Clobber]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86X64CallAlloc::clobber() {
  VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  uint32_t affected = _vi->_clobberedRegs.get(C) & state->_occupied.get(C);

  for (i = 0; affected != 0; i++, affected >>= 1) {
    if (affected & 0x1) {
      VarData* vd = sVars[i];
      ASMJIT_ASSERT(vd != NULL);

      VarAttr* va = vd->getVa();
      uint32_t vdState = kVarStateUnused;

      if (!vd->isModified() || (va != NULL && (va->getFlags() & (kVarAttrOutAll | kVarAttrUnuse)) != 0)) {
        vdState = kVarStateMem;
      }

      _context->unuse<C>(vd, vdState);
    }
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64CallAlloc - Ret]
// ============================================================================

ASMJIT_INLINE void X86X64CallAlloc::ret() {
  X86X64CallNode* node = getNode();
  X86X64FuncDecl* decl = node->getDecl();

  uint32_t i;
  Operand* retList = node->_ret;

  for (i = 0; i < 2; i++) {
    const FuncInOut& ret = decl->getRet(i);
    Operand* op = &retList[i];

    if (!ret.hasRegIndex() || !op->isVar())
      continue;

    VarData* vd = _compiler->getVdById(op->getId());
    uint32_t regIndex = ret.getRegIndex();

    switch (vd->getClass()) {
      case kRegClassGp:
        if (vd->getRegIndex() != kInvalidReg)
          _context->unuse<kRegClassGp>(vd);
        _context->attach<kRegClassGp>(vd, regIndex, true);
        break;
      case kRegClassMm:
        if (vd->getRegIndex() != kInvalidReg)
          _context->unuse<kRegClassMm>(vd);
        _context->attach<kRegClassMm>(vd, regIndex, true);
        break;
      case kRegClassXy:
        if (vd->getRegIndex() != kInvalidReg)
          _context->unuse<kRegClassXy>(vd);
        _context->attach<kRegClassXy>(vd, regIndex, true);
        break;
    }
  }
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - TranslateOperands]
// ============================================================================

//! @internal
static Error X86X64Context_translateOperands(X86X64Context* self, Operand* opList, uint32_t opCount) {
  X86X64Compiler* compiler = self->getCompiler();
  const VarInfo* varInfo = _varInfo;

  uint32_t hasGpdBase = compiler->getRegSize() == 4;

  // Translate variables into registers.
  for (uint32_t i = 0; i < opCount; i++) {
    Operand* op = &opList[i];

    if (op->isVar()) {
      VarData* vd = compiler->getVdById(op->getId());
      ASMJIT_ASSERT(vd != NULL);
      ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

      op->_vreg.op = kOperandTypeReg;
      op->_vreg.index = vd->getRegIndex();
    }
    else if (op->isMem()) {
      Mem* m = static_cast<Mem*>(op);

      if (m->isBaseIndexType() && OperandUtil::isVarId(m->getBase())) {
        VarData* vd = compiler->getVdById(m->getBase());

        if (m->getMemType() == kMemTypeBaseIndex) {
          ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);
          op->_vmem.base = vd->getRegIndex();
        }
        else {
          if (!vd->isMemArg())
            self->getVarCell(vd);

          // Offset will be patched later by X86X64Context_patchFuncMem().
          m->setGpdBase(hasGpdBase);
          m->adjust(vd->isMemArg() ? self->_argActualDisp : self->_varActualDisp);
        }
      }

      if (OperandUtil::isVarId(m->getIndex())) {
        VarData* vd = compiler->getVdById(m->getIndex());
        ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);
        ASMJIT_ASSERT(vd->getRegIndex() != kRegIndexR12);
        op->_vmem.index = vd->getRegIndex();
      }
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - TranslatePrologEpilog]
// ============================================================================

//! @internal
static Error X86X64Context_initFunc(X86X64Context* self, X86X64FuncNode* func) {
  X86X64Compiler* compiler = self->getCompiler();
  X86X64FuncDecl* decl = func->getDecl();

  RegMask& clobberedRegs = self->_clobberedRegs;
  uint32_t regSize = compiler->getRegSize();

  // Setup "Save-Restore" registers.
  func->_saveRestoreRegs.set(kRegClassGp, clobberedRegs.get(kRegClassGp) & decl->getPreserved(kRegClassGp));
  func->_saveRestoreRegs.set(kRegClassFp, 0                                                               );
  func->_saveRestoreRegs.set(kRegClassMm, clobberedRegs.get(kRegClassMm) & decl->getPreserved(kRegClassMm));
  func->_saveRestoreRegs.set(kRegClassXy, clobberedRegs.get(kRegClassXy) & decl->getPreserved(kRegClassXy));

  ASMJIT_ASSERT(!func->_saveRestoreRegs.has(kRegClassGp, IntUtil::mask(kRegIndexSp)));

  // Setup required stack alignment and kFuncFlagIsStackMisaligned.
  {
    uint32_t requiredStackAlignment = IntUtil::iMax(self->_memMaxAlign, self->getRegSize());

    if (requiredStackAlignment < 16) {
      // Require 16-byte alignment 8-byte vars are used.
      if (self->_mem8ByteVarsUsed)
        requiredStackAlignment = 16;
      else if (func->_saveRestoreRegs.get(kRegClassMm) || func->_saveRestoreRegs.get(kRegClassXy))
        requiredStackAlignment = 16;
      else if (IntUtil::inInterval<uint32_t>(func->getRequiredStackAlignment(), 8, 16))
        requiredStackAlignment = 16;
    }

    if (func->getRequiredStackAlignment() < requiredStackAlignment)
      func->setRequiredStackAlignment(requiredStackAlignment);

    func->updateRequiredStackAlignment();
  }

  // Adjust stack pointer if function is caller.
  if (func->isCaller()) {
    func->addFuncFlags(kFuncFlagIsStackAdjusted);
  }

  // Adjust stack pointer if manual stack alignment is needed.
  if (func->isStackMisaligned() && func->isNaked()) {
    // Get a memory cell where the original stack frame will be stored.
    MemCell* cell = self->_newStackCell(regSize, regSize);
    if (cell == NULL)
      return self->getError();

    func->addFuncFlags(kFuncFlagIsStackAdjusted);
    self->_stackFrameCell = cell;

    if (decl->getArgStackSize() > 0) {
      func->addFuncFlags(kFuncFlagMoveArgs);
      func->setExtraStackSize(decl->getArgStackSize());
    }

    // Get temporary register which will be used to align the stack frame.
    uint32_t fRegMask = IntUtil::bits(self->_baseRegsCount);
    uint32_t stackFrameCopyRegs;

    fRegMask &= ~(decl->getUsed(kRegClassGp) | IntUtil::mask(kRegIndexSp));
    stackFrameCopyRegs = fRegMask;

    // Try to remove modified registers from the mask.
    uint32_t tRegMask = fRegMask & ~self->getClobberedRegs(kRegClassGp);
    if (tRegMask != 0)
      fRegMask = tRegMask;

    // Try to remove preserved registers from the mask.
    tRegMask = fRegMask & decl->getPreserved(kRegClassGp);
    if (tRegMask != 0)
      fRegMask = tRegMask;

    ASMJIT_ASSERT(fRegMask != 0);

    uint32_t fRegIndex = IntUtil::findFirstBit(fRegMask);
    func->_stackFrameRegIndex = static_cast<uint8_t>(fRegIndex);

    // We have to save the register on the stack (it will be the part of prolog
    // and epilog), however we shouldn't save it twice, so we will remove it
    // from '_saveRestoreRegs' in case that it is preserved.
    fRegMask = IntUtil::mask(fRegIndex);
    if ((fRegMask & decl->getPreserved(kRegClassGp)) != 0) {
      func->_saveRestoreRegs.del(kRegClassGp, fRegMask);
      func->_isStackFrameRegPreserved = true;
    }

    if (func->hasFuncFlag(kFuncFlagMoveArgs)) {
      uint32_t maxRegs = (func->getArgStackSize() + regSize - 1) / regSize;
      stackFrameCopyRegs &= ~fRegMask;

      tRegMask = stackFrameCopyRegs & self->getClobberedRegs(kRegClassGp);
      uint32_t tRegCnt = IntUtil::bitCount(tRegMask);

      if (tRegCnt > 1 || (tRegCnt > 0 && tRegCnt <= maxRegs))
        stackFrameCopyRegs = tRegMask;
      else
        stackFrameCopyRegs = IntUtil::keepNOnesFromRight(stackFrameCopyRegs, IntUtil::iMin<uint32_t>(maxRegs, 2));

      func->_saveRestoreRegs.add(kRegClassGp, stackFrameCopyRegs & decl->getPreserved(kRegClassGp));
      IntUtil::indexNOnesFromRight(func->_stackFrameCopyGpIndex, stackFrameCopyRegs, maxRegs);
    }
  }
  // If function is not naked we generate standard "EBP/RBP" stack frame.
  else if (!func->isNaked()) {
    uint32_t fRegIndex = kRegIndexBp;

    func->_stackFrameRegIndex = static_cast<uint8_t>(fRegIndex);
    func->_isStackFrameRegPreserved = true;
  }

  ASMJIT_PROPAGATE_ERROR(self->resolveCellOffsets());

  // Adjust stack pointer if requested memory can't fit into "Red Zone" or "Spill Zone".
  if (self->_memAllTotal > IntUtil::iMax<uint32_t>(func->getRedZoneSize(), func->getSpillZoneSize())) {
    func->addFuncFlags(kFuncFlagIsStackAdjusted);
  }

  // Setup stack size used to save preserved registers.
  {
    uint32_t memGpSize  = IntUtil::bitCount(func->_saveRestoreRegs.get(kRegClassGp)) * regSize;
    uint32_t memMmSize  = IntUtil::bitCount(func->_saveRestoreRegs.get(kRegClassMm)) * 8;
    uint32_t memXmmSize = IntUtil::bitCount(func->_saveRestoreRegs.get(kRegClassXy)) * 16;

    if (func->hasFuncFlag(kFuncFlagPushPop)) {
      func->_pushPopStackSize = memGpSize;
      func->_moveStackSize = memXmmSize + IntUtil::alignTo<uint32_t>(memMmSize, 16);
    }
    else {
      func->_pushPopStackSize = 0;
      func->_moveStackSize = memXmmSize + IntUtil::alignTo<uint32_t>(memMmSize + memGpSize, 16);
    }
  }

  // Setup adjusted stack size.
  if (func->isStackMisaligned()) {
    func->_alignStackSize = 0;
  }
  else {
    // If function is aligned, the RETURN address is stored to the aligned
    // [ZSP - PtrSize] which makes current ZSP unaligned.
    int32_t v = regSize;

    // If we have to store function frame pointer we have to count it as well,
    // because it is the first thing pushed on the stack.
    if (func->hasStackFrameReg() && func->isStackFrameRegPreserved())
      v += regSize;

    // Count push/pop sequence.
    v += func->getPushPopStackSize();

    // Calculate the final offset to keep stack alignment.
    func->_alignStackSize = IntUtil::deltaTo<uint32_t>(v, func->getRequiredStackAlignment());
  }

  // Memory stack size.
  func->_memStackSize = self->_memAllTotal;
  func->_alignedMemStackSize = IntUtil::alignTo<uint32_t>(func->_memStackSize, func->_requiredStackAlignment);

  if (func->isNaked()) {
    self->_argBaseReg = kRegIndexSp;

    if (func->isStackAdjusted()) {
      if (func->isStackMisaligned()) {
        self->_argBaseOffset = static_cast<int32_t>(
          func->getCallStackSize() +
          func->getAlignedMemStackSize() +
          func->getMoveStackSize() +
          func->getAlignStackSize());
        self->_argBaseOffset -= regSize;
      }
      else {
        self->_argBaseOffset = static_cast<int32_t>(
          func->getCallStackSize() +
          func->getAlignedMemStackSize() +
          func->getMoveStackSize() +
          func->getPushPopStackSize() +
          func->getExtraStackSize() +
          func->getAlignStackSize());
      }
    }
    else {
      self->_argBaseOffset = func->getPushPopStackSize();
    }
  }
  else {
    self->_argBaseReg = kRegIndexBp;
    self->_argBaseOffset = regSize; // Caused by "push zbp".
  }

  self->_varBaseReg = kRegIndexSp;
  self->_varBaseOffset = func->getCallStackSize();

  if (!func->isStackAdjusted()) {
    self->_varBaseOffset = -static_cast<int32_t>(
      func->_alignStackSize +
      func->_alignedMemStackSize +
      func->_moveStackSize);
  }

  return kErrorOk;
}

//! @internal
static Error X86X64Context_patchFuncMem(X86X64Context* self, X86X64FuncNode* func, BaseNode* stop) {
  X86X64Compiler* compiler = self->getCompiler();
  BaseNode* node = func;

  do {
    if (node->getType() == kNodeTypeInst) {
      InstNode* iNode = static_cast<InstNode*>(node);

      if (iNode->hasMemOp()) {
        Mem* m = iNode->getMemOp<Mem>();

        if (m->getMemType() == kMemTypeStackIndex && OperandUtil::isVarId(m->getBase())) {
          VarData* vd = compiler->getVdById(m->getBase());
          ASMJIT_ASSERT(vd != NULL);

          if (vd->isMemArg()) {
            m->_vmem.base = self->_argBaseReg;
            m->_vmem.displacement += vd->getMemOffset();
            m->_vmem.displacement += self->_argBaseOffset;
          }
          else {
            MemCell* cell = vd->getMemCell();
            ASMJIT_ASSERT(cell != NULL);

            m->_vmem.base = self->_varBaseReg;
            m->_vmem.displacement += cell->getOffset();
            m->_vmem.displacement += self->_varBaseOffset;
          }
        }
      }
    }

    node = node->getNext();
  } while (node != stop);

  return kErrorOk;
}

//! @internal
static Error X86X64Context_translatePrologEpilog(X86X64Context* self, X86X64FuncNode* func) {
  X86X64Compiler* compiler = self->getCompiler();
  X86X64FuncDecl* decl = func->getDecl();

  uint32_t regSize = compiler->getRegSize();

  int32_t stackSize = static_cast<int32_t>(
    func->getAlignStackSize() +
    func->getCallStackSize() +
    func->getAlignedMemStackSize() +
    func->getMoveStackSize() +
    func->getExtraStackSize());
  int32_t stackAlignment = func->getRequiredStackAlignment();

  int32_t stackBase;
  int32_t stackPtr;

  if (func->isStackAdjusted()) {
    stackBase = static_cast<int32_t>(
      func->getCallStackSize() +
      func->getAlignedMemStackSize());
  }
  else {
    stackBase = -static_cast<int32_t>(
      func->getAlignedMemStackSize() +
      func->getAlignStackSize() +
      func->getExtraStackSize());
  }

  uint32_t i, mask;
  uint32_t regsGp  = func->getSaveRestoreRegs(kRegClassGp);
  uint32_t regsMm  = func->getSaveRestoreRegs(kRegClassMm);
  uint32_t regsXmm = func->getSaveRestoreRegs(kRegClassXy);

  bool earlyPushPop = false;
  bool useLeaEpilog = false;

  GpReg gpReg(self->_zsp);
  GpReg fpReg(self->_zbp);

  Mem fpOffset;

  // --------------------------------------------------------------------------
  // [Prolog]
  // --------------------------------------------------------------------------

  compiler->_setCursor(func->getEntryNode());

  if (compiler->getLogger())
    compiler->comment("Prolog");

  // Entry.
  if (func->isNaked()) {
    if (func->isStackMisaligned()) {
      fpReg.setIndex(func->getStackFrameRegIndex());
      fpOffset = ptr(self->_zsp, static_cast<int32_t>(self->_stackFrameCell->getOffset()));

      earlyPushPop = func->hasFuncFlag(kFuncFlagPushPop);
      if (earlyPushPop)
        self->emitPushSequence(regsGp);

      if (func->isStackFrameRegPreserved())
        compiler->emit(kInstPush, fpReg);

      compiler->emit(kInstMov, fpReg, self->_zsp);
    }
  }
  else {
    compiler->emit(kInstPush, fpReg);
    compiler->emit(kInstMov, fpReg, self->_zsp);
  }

  if (func->hasFuncFlag(kFuncFlagPushPop) && !earlyPushPop) {
    self->emitPushSequence(regsGp);
    if (func->isStackMisaligned() && regsGp != 0)
      useLeaEpilog = true;
  }

  // Adjust stack pointer.
  if (func->isStackAdjusted()) {
    stackBase = static_cast<int32_t>(func->getAlignedMemStackSize() + func->getCallStackSize());

    if (stackSize)
      compiler->emit(kInstSub, self->_zsp, stackSize);

    if (func->isStackMisaligned())
      compiler->emit(kInstAnd, self->_zsp, -stackAlignment);

    if (func->isStackMisaligned() && func->isNaked())
      compiler->emit(kInstMov, fpOffset, fpReg);
  }
  else {
    stackBase = -static_cast<int32_t>(func->getAlignStackSize() + func->getMoveStackSize());
  }

  // Save Xmm/Mm/Gp (Mov).
  stackPtr = stackBase;
  for (i = 0, mask = regsXmm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kInstMovaps, oword_ptr(self->_zsp, stackPtr), xmm(i));
      stackPtr += 16;
    }
  }

  for (i = 0, mask = regsMm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kInstMovq, qword_ptr(self->_zsp, stackPtr), mm(i));
      stackPtr += 8;
    }
  }

  if (!func->hasFuncFlag(kFuncFlagPushPop)) {
    for (i = 0, mask = regsGp; mask != 0; i++, mask >>= 1) {
      if (mask & 0x1) {
        compiler->emit(kInstMov, ptr(self->_zsp, stackPtr), gpReg.setIndex(i));
        stackPtr += regSize;
      }
    }
  }

  // --------------------------------------------------------------------------
  // [Copy-Args]
  // --------------------------------------------------------------------------

  if (func->hasFuncFlag(kFuncFlagMoveArgs)) {
    uint32_t argStackPos = 0;
    uint32_t argStackSize = decl->getArgStackSize();

    uint32_t moveIndex = 0;
    uint32_t moveCount = (argStackSize + regSize - 1) / regSize;

    GpReg r[8];
    uint32_t numRegs = 0;

    for (i = 0; i < 6; i++)
      if (func->_stackFrameCopyGpIndex[i] != kInvalidReg)
        r[numRegs++] = gpReg.setIndex(func->_stackFrameCopyGpIndex[i]);

    int32_t dSrc = func->getPushPopStackSize() + regSize;
    int32_t dDst = func->getAlignStackSize() +
                   func->getCallStackSize() +
                   func->getAlignedMemStackSize() +
                   func->getMoveStackSize();

    if (func->isStackFrameRegPreserved())
      dSrc += regSize;

    Mem mSrc = ptr(fpReg, dSrc);
    Mem mDst = ptr(self->_zsp, dDst);

    while (moveIndex < moveCount) {
      uint32_t numMovs = IntUtil::iMin<uint32_t>(moveCount - moveIndex, numRegs);

      for (i = 0; i < numMovs; i++)
        compiler->emit(kInstMov, r[i], mSrc.adjusted((moveIndex + i) * regSize));
      for (i = 0; i < numMovs; i++)
        compiler->emit(kInstMov, mDst.adjusted((moveIndex + i) * regSize), r[i]);

      argStackPos += numMovs * regSize;
      moveIndex += numMovs;
    }
  }

  if (compiler->getLogger())
    compiler->comment("Body");

  // --------------------------------------------------------------------------
  // [Epilog]
  // --------------------------------------------------------------------------

  compiler->_setCursor(func->getExitNode());

  if (compiler->getLogger())
    compiler->comment("Epilog");

  // Restore Xmm/Mm/Gp (Mov).
  stackPtr = stackBase;
  for (i = 0, mask = regsXmm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kInstMovaps, xmm(i), oword_ptr(self->_zsp, stackPtr));
      stackPtr += 16;
    }
  }

  for (i = 0, mask = regsMm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kInstMovq, mm(i), qword_ptr(self->_zsp, stackPtr));
      stackPtr += 8;
    }
  }

  if (!func->hasFuncFlag(kFuncFlagPushPop)) {
    for (i = 0, mask = regsGp; mask != 0; i++, mask >>= 1) {
      if (mask & 0x1) {
        compiler->emit(kInstMov, gpReg.setIndex(i), ptr(self->_zsp, stackPtr));
        stackPtr += regSize;
      }
    }
  }

  // Adjust stack.
  if (useLeaEpilog) {
    compiler->emit(kInstLea, self->_zsp, ptr(fpReg, -static_cast<int32_t>(func->getPushPopStackSize())));
  }
  else if (!func->isStackMisaligned()) {
    if (func->isStackAdjusted() && stackSize != 0)
      compiler->emit(kInstAdd, self->_zsp, stackSize);
  }

  // Restore Gp (Push/Pop).
  if (func->hasFuncFlag(kFuncFlagPushPop) && !earlyPushPop)
    self->emitPopSequence(regsGp);

  // Emms.
  if (func->hasFuncFlag(kFuncFlagEmms))
    compiler->emit(kInstEmms);

  // MFence/SFence/LFence.
  if (func->hasFuncFlag(kFuncFlagSFence) & func->hasFuncFlag(kFuncFlagLFence))
    compiler->emit(kInstMfence);
  else if (func->hasFuncFlag(kFuncFlagSFence))
    compiler->emit(kInstSfence);
  else if (func->hasFuncFlag(kFuncFlagLFence))
    compiler->emit(kInstLfence);

  // Leave.
  if (func->isNaked()) {
    if (func->isStackMisaligned()) {
      compiler->emit(kInstMov, self->_zsp, fpOffset);

      if (func->isStackFrameRegPreserved())
        compiler->emit(kInstPop, fpReg);

      if (earlyPushPop)
        self->emitPopSequence(regsGp);
    }
  }
  else {
    if (useLeaEpilog) {
      compiler->emit(kInstPop, fpReg);
    }
    else if (func->hasFuncFlag(kFuncFlagLeave)) {
      compiler->emit(kInstLeave);
    }
    else {
      compiler->emit(kInstMov, self->_zsp, fpReg);
      compiler->emit(kInstPop, fpReg);
    }
  }

  // Emit return.
  if (decl->getCalleePopsStack())
    compiler->emit(kInstRet, static_cast<int32_t>(decl->getArgStackSize()));
  else
    compiler->emit(kInstRet);

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - TranslateJump]
// ============================================================================

//! @internal
static void X86X64Context_translateJump(X86X64Context* self, JumpNode* jNode, TargetNode* jTarget) {
  X86X64Compiler* compiler = self->getCompiler();
  BaseNode* extNode = self->getExtraBlock();

  // TODO: [COMPILER] State Change.
  compiler->_setCursor(extNode);
  self->switchState(jTarget->getState());

  // If any instruction was added during switchState() we have to wrap the
  // generated code in a block.
  if (compiler->getCursor() != extNode) {
    TargetNode* jTrampolineTarget = compiler->newTarget();

    // Add the jump to the target.
    compiler->jmp(jTarget->getLabel());

    // Add the trampoline-label we jump to change the state.
    extNode = compiler->setCursor(extNode);
    compiler->addNode(jTrampolineTarget);

    // Finally, patch the jump target.
    ASMJIT_ASSERT(jNode->getOpCount() > 0);
    jNode->_opList[0] = jTrampolineTarget->getLabel();
    jNode->_target = jTrampolineTarget;
  }

  // Store the extNode and load the state back.
  self->setExtraBlock(extNode);
  self->loadState(jNode->_state);
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - TranslateRet]
// ============================================================================

static Error X86X64Context_translateRet(X86X64Context* self, RetNode* rNode, TargetNode* exitTarget) {
  BaseNode* node = rNode->getNext();

  while (node != NULL) {
    switch (node->getType()) {
      // If we have found an exit label we just return, there is no need to
      // emit jump to that.
      case kNodeTypeTarget:
        if (static_cast<TargetNode*>(node) == exitTarget)
          return kErrorOk;
        goto _EmitRet;

      case kNodeTypeEmbed:
      case kNodeTypeInst:
      case kNodeTypeCall:
      case kNodeTypeRet:
        goto _EmitRet;

      // Continue iterating.
      case kNodeTypeComment:
      case kNodeTypeAlign:
      case kNodeTypeHint:
        break;

      // Invalid node to be here.
      case kNodeTypeFunc:
        return self->getCompiler()->setError(kErrorInvalidState);

      // We can't go forward from here.
      case kNodeTypeEnd:
        return kErrorOk;
    }

    node = node->getNext();
  }

_EmitRet:
  {
    X86X64Compiler* compiler = self->getCompiler();

    compiler->_setCursor(rNode);
    compiler->jmp(exitTarget->getLabel());
  }
  return kErrorOk;
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Translate - Func]
// ============================================================================

Error X86X64Context::translate() {
  X86X64Compiler* compiler = getCompiler();
  X86X64FuncNode* func = getFunc();

  // Register allocator contexts.
  X86X64VarAlloc vAlloc(this);
  X86X64CallAlloc cAlloc(this);

  // Flow.
  BaseNode* node_ = func;
  BaseNode* next = NULL;
  BaseNode* stop = getStop();

  PodList<BaseNode*>::Link* jLink = _jccList.getFirst();

  for (;;) {
    while (node_->isTranslated()) {
      // Switch state if we went to the already translated node.
      if (node_->getType() == kNodeTypeTarget) {
        TargetNode* node = static_cast<TargetNode*>(node_);
        compiler->_setCursor(node->getPrev());
        switchState(node->getState());
      }

_NextGroup:
      if (jLink == NULL) {
        goto _Done;
      }
      else {
        JumpNode* jNode = static_cast<JumpNode*>(jLink->getValue());
        jLink = jLink->getNext();

        BaseNode* jFlow = X86X64Context_getOppositeJccFlow(jNode);
        loadState(jNode->getState());

        // TODO:
        if (jNode->getNext() == jFlow) {
        }
        else {
          X86X64Context_translateJump(this, jNode, static_cast<TargetNode*>(jFlow));
        }

        node_ = jFlow;
        if (node_->isTranslated())
          goto _NextGroup;
      }
    }

    next = node_->getNext();
    node_->addFlags(kNodeFlagIsTranslated);

    switch (node_->getType()) {
      // ----------------------------------------------------------------------
      // [Align / Embed]
      // ----------------------------------------------------------------------

      case kNodeTypeAlign:
      case kNodeTypeEmbed:
        break;

      // ----------------------------------------------------------------------
      // [Target]
      // ----------------------------------------------------------------------

      case kNodeTypeTarget: {
        TargetNode* node = static_cast<TargetNode*>(node_);
        ASMJIT_ASSERT(!node->hasState());
        node->setState(saveState());
        break;
      }

      // ----------------------------------------------------------------------
      // [Inst/Call/SArg/Ret]
      // ----------------------------------------------------------------------

      case kNodeTypeInst:
      case kNodeTypeCall:
      case kNodeTypeSArg:
        // Update VarAttr's unuse flags based on liveness of the next node.
        if (!node_->isJcc()) {
          VarInst* vi = static_cast<VarInst*>(node_->getVarInst());
          VarBits* liveness = next->getLiveness();

          if (vi != NULL && liveness != NULL) {
            VarAttr* vaList = vi->getVaList();
            uint32_t vaCount = vi->getVaCount();

            for (uint32_t i = 0; i < vaCount; i++) {
              VarAttr* va = &vaList[i];
              VarData* vd = va->getVd();

              if (!liveness->getBit(vd->getContextId()))
                va->addFlags(kVarAttrUnuse);
            }
          }
        }

        if (node_->getType() == kNodeTypeCall) {
          ASMJIT_PROPAGATE_ERROR(cAlloc.run(static_cast<X86X64CallNode*>(node_)));
          break;
        }
        // ... Fall through ...

      case kNodeTypeHint:
      case kNodeTypeRet: {
        ASMJIT_PROPAGATE_ERROR(vAlloc.run(node_));

        // Handle conditional/unconditional jump.
        if (node_->isJmpOrJcc()) {
          JumpNode* node = static_cast<JumpNode*>(node_);
          TargetNode* jTarget = node->getTarget();

          if (node->isJmp()) {
            if (jTarget->hasState()) {
              compiler->_setCursor(node->getPrev());
              switchState(jTarget->getState());

              goto _NextGroup;
            }
            else {
              next = jTarget;
            }
          }
          else {
            BaseNode* jNext = node->getNext();

            if (jTarget->isTranslated()) {
              if (jNext->isTranslated()) {
                ASMJIT_ASSERT(jNext->getType() == kNodeTypeTarget);
                // TODO: [COMPILER] State - Do intersection of two states if possible.
              }

              BaseVarState* savedState = saveState();
              node->setState(savedState);

              X86X64Context_translateJump(this, node, jTarget);
              next = jNext;
            }
            else if (jNext->isTranslated()) {
              ASMJIT_ASSERT(jNext->getType() == kNodeTypeTarget);

              BaseVarState* savedState = saveState();
              node->setState(savedState);

              compiler->_setCursor(node);
              switchState(static_cast<TargetNode*>(jNext)->getState());

              next = jTarget;
            }
            else {
              node->setState(saveState());
              next = X86X64Context_getJccFlow(node);
            }
          }
        }
        else if (node_->isRet()) {
          ASMJIT_PROPAGATE_ERROR(
            X86X64Context_translateRet(this, static_cast<RetNode*>(node_), func->getExitNode()));
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Func]
      // ----------------------------------------------------------------------

      case kNodeTypeFunc: {
        ASMJIT_ASSERT(node_ == func);

        X86X64FuncDecl* decl = func->getDecl();
        VarInst* vi = func->getVarInst<VarInst>();

        if (vi != NULL) {
          uint32_t i;
          uint32_t argCount = func->_x86Decl.getArgCount();

          for (i = 0; i < argCount; i++) {
            const FuncInOut& arg = decl->getArg(i);

            VarData* vd = func->getArg(i);
            VarAttr* va = vi->findVa(vd);
            ASMJIT_ASSERT(va != NULL);

            if (vd->getFlags() & kVarAttrUnuse)
              continue;

            // Special means that the argument is passed in register.
            uint32_t regIndex = va->getOutRegIndex();
            if (regIndex != kInvalidReg && (va->getFlags() & kVarAttrOutConv) == 0) {
              switch (vd->getClass()) {
                case kRegClassGp: attach<kRegClassGp>(vd, regIndex, true); break;
                case kRegClassMm: attach<kRegClassMm>(vd, regIndex, true); break;
                case kRegClassXy: attach<kRegClassXy>(vd, regIndex, true); break;
              }
            }
            else if (va->hasFlag(kVarAttrOutConv)) {
              // TODO: [COMPILER] Function Argument Conversion.
            }
            else {
              vd->_isMemArg = true;
              vd->setMemOffset(arg.getStackOffset());
              vd->setState(kVarStateMem);
            }
          }
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [End]
      // ----------------------------------------------------------------------

      case kNodeTypeEnd: {
        goto _NextGroup;
      }

      default:
        break;
    }

    if (next == stop)
      goto _NextGroup;
    node_ = next;
  }

_Done:
  ASMJIT_PROPAGATE_ERROR(X86X64Context_initFunc(this, func));
  ASMJIT_PROPAGATE_ERROR(X86X64Context_patchFuncMem(this, func, stop));
  ASMJIT_PROPAGATE_ERROR(X86X64Context_translatePrologEpilog(this, func));

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86x64::X86X64Context - Serialize]
// ============================================================================

template<int LoggingEnabled>
static ASMJIT_INLINE Error X86X64Context_serialize(X86X64Context* self, X86X64Assembler* assembler, BaseNode* start, BaseNode* stop) {
  BaseNode* node_ = start;
  StringBuilder& sb = self->_stringBuilder;

  BaseLogger* logger;
  const char* comment;

  if (LoggingEnabled) {
    logger = assembler->getLogger();
  }

  // Create labels on Assembler side.
  ASMJIT_PROPAGATE_ERROR(
    assembler->_registerIndexedLabels(self->getCompiler()->_targets.getLength()));

  do {
    if (LoggingEnabled) {
      comment = node_->getComment();

      if (node_->hasLiveness()) {
        uint32_t i;
        uint32_t vdCount = static_cast<uint32_t>(self->_contextVd.getLength());

        VarBits* liveness = node_->getLiveness();
        VarInst* vi = static_cast<VarInst*>(node_->getVarInst());

        sb.clear();
        sb.appendChars(' ', vdCount);

        for (i = 0; i < vdCount; i++) {
          if (liveness->getBit(i))
            sb.getData()[i] = '.';
        }

        if (vi != NULL) {
          uint32_t vaCount = vi->getVaCount();

          for (i = 0; i < vaCount; i++) {
            VarAttr* va = vi->getVa(i);
            VarData* vd = va->getVd();

            uint32_t flags = va->getFlags();
            char c = 'u';

            if ( (flags & kVarAttrInAll) && !(flags & kVarAttrOutAll)) c  = 'r';
            if (!(flags & kVarAttrInAll) &&  (flags & kVarAttrOutAll)) c  = 'w';
            if ( (flags & kVarAttrInAll) &&  (flags & kVarAttrOutAll)) c  = 'x';

            if ((flags & kVarAttrUnuse))
              c -= 'a' - 'A';

            sb.getData()[vd->getContextId()] = c;
          }
        }

        assembler->_comment = sb.getData();
      }
      else {
        assembler->_comment = comment;
      }
    }

    switch (node_->getType()) {
      case kNodeTypeAlign: {
        AlignNode* node = static_cast<AlignNode*>(node_);
        assembler->align(node->getSize());
        break;
      }

      case kNodeTypeEmbed: {
        EmbedNode* node = static_cast<EmbedNode*>(node_);
        assembler->embed(node->getData(), node->getSize());
        break;
      }

      case kNodeTypeComment: {
        CommentNode* node = static_cast<CommentNode*>(node_);
        if (LoggingEnabled) {
          logger->logFormat(kLoggerStyleComment,
            "%s; %s\n", logger->getIndentation(), node->getComment());
        }
        break;
      }

      case kNodeTypeHint: {
        break;
      }

      case kNodeTypeTarget: {
        TargetNode* node = static_cast<TargetNode*>(node_);
        assembler->bind(node->getLabel());
        break;
      }

      case kNodeTypeInst: {
        InstNode* node = static_cast<InstNode*>(node_);

        uint32_t code = node->getCode();
        uint32_t opCount = node->getOpCount();

        const Operand* opList = node->getOpList();
        assembler->_options = node->getOptions();

        const Operand* o0 = &noOperand;
        const Operand* o1 = &noOperand;
        const Operand* o2 = &noOperand;

        if (node->isSpecial()) {
          switch (code) {
            case kInstCpuid:
              break;

            case kInstCbw:
            case kInstCdq:
            case kInstCdqe:
            case kInstCwd:
            case kInstCwde:
            case kInstCqo:
              break;

            case kInstCmpxchg:
              o0 = &opList[1];
              o1 = &opList[2];
              break;

            case kInstCmpxchg8b :
            case kInstCmpxchg16b:
              o0 = &opList[4];
              break;

            case kInstDaa:
            case kInstDas:
              break;

            case kInstImul:
            case kInstMul:
            case kInstIdiv:
            case kInstDiv:
              // We assume "Mul/Div dst_hi (implicit), dst_lo (implicit), src (explicit)".
              ASMJIT_ASSERT(opCount == 3);
              o0 = &opList[2];
              break;

            case kInstMovptr:
              break;

            case kInstLahf:
            case kInstSahf:
              break;

            case kInstMaskmovq:
            case kInstMaskmovdqu:
              o0 = &opList[1];
              o1 = &opList[2];
              break;

            case kInstEnter:
              o0 = &opList[0];
              o1 = &opList[1];
              break;

            case kInstLeave:
              break;

            case kInstRet:
              if (opCount > 0)
                o0 = &opList[0];
              break;

            case kInstMonitor:
            case kInstMwait:
              break;

            case kInstPop:
              o0 = &opList[0];
              break;

            case kInstPopa:
            case kInstPopf:
              break;

            case kInstPush:
              o0 = &opList[0];
              break;

            case kInstPusha:
            case kInstPushf:
              break;

            case kInstRcl:
            case kInstRcr:
            case kInstRol:
            case kInstRor:
            case kInstSal:
            case kInstSar:
            case kInstShl:
            case kInstShr:
              o0 = &opList[0];
              o1 = &cl;
              break;

            case kInstShld:
            case kInstShrd:
              o0 = &opList[0];
              o1 = &opList[1];
              o2 = &cl;
              break;

            case kInstRdtsc:
            case kInstRdtscp:
              break;

            case kInstRepLodsb  : case kInstRepLodsd  : case kInstRepLodsq  : case kInstRepLodsw  :
            case kInstRepMovsb  : case kInstRepMovsd  : case kInstRepMovsq  : case kInstRepMovsw  :
            case kInstRepStosb  : case kInstRepStosd  : case kInstRepStosq  : case kInstRepStosw  :
            case kInstRepeCmpsb : case kInstRepeCmpsd : case kInstRepeCmpsq : case kInstRepeCmpsw :
            case kInstRepeScasb : case kInstRepeScasd : case kInstRepeScasq : case kInstRepeScasw :
            case kInstRepneCmpsb: case kInstRepneCmpsd: case kInstRepneCmpsq: case kInstRepneCmpsw:
            case kInstRepneScasb: case kInstRepneScasd: case kInstRepneScasq: case kInstRepneScasw:
              break;

            default:
              ASMJIT_ASSERT(!"Reached");
          }
        }
        else {
          if (opCount > 0) o0 = &opList[0];
          if (opCount > 1) o1 = &opList[1];
          if (opCount > 2) o2 = &opList[2];
        }

        // We use this form, because it is the main one.
        assembler->emit(code, *o0, *o1, *o2);
        break;
      }

      // Function scope and return is translated to another nodes, no special
      // handling is required at this point.
      case kNodeTypeFunc:
      case kNodeTypeEnd:
      case kNodeTypeRet: {
        break;
      }

      // Function call adds nodes before and after, but it's required to emit
      // the call instruction by itself.
      case kNodeTypeCall: {
        X86X64CallNode* node = static_cast<X86X64CallNode*>(node_);
        assembler->emit(kInstCall, node->_target, noOperand, noOperand);
        break;
      }

      default:
        break;
    }

    node_ = node_->getNext();
  } while (node_ != stop);

  return kErrorOk;
}

Error X86X64Context::serialize(BaseAssembler* assembler, BaseNode* start, BaseNode* stop) {
  if (!assembler->hasLogger())
    return X86X64Context_serialize<0>(this, static_cast<X86X64Assembler*>(assembler), start, stop);
  else
    return X86X64Context_serialize<1>(this, static_cast<X86X64Assembler*>(assembler), start, stop);
}

} // x86x64 namespace
} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
