// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER) && (defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64))

// [Dependencies - AsmJit]
#include "../base/containers.h"
#include "../base/cpuinfo.h"
#include "../base/utils.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86compilercontext_p.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

static Error X86Context_translateOperands(X86Context* self, Operand* opList, uint32_t opCount);

// ============================================================================
// [asmjit::X86Context - Utils]
// ============================================================================

// Getting `VarClass` is the only safe operation when dealing with denormalized
// `varType`. Any other property would require to map vType to the architecture
// specific type.
static ASMJIT_INLINE uint32_t x86VarTypeToClass(uint32_t vType) noexcept {
  ASMJIT_ASSERT(vType < kX86VarTypeCount);
  return _x86VarInfo[vType].getRegClass();
}

// ============================================================================
// [asmjit::X86Context - Annotate]
// ============================================================================

// Annotation is also used by ASMJIT_TRACE.
#if !defined(ASMJIT_DISABLE_LOGGER)
static void X86Context_annotateVariable(X86Context* self,
  StringBuilder& sb, const VarData* vd) {

  const char* name = vd->getName();
  if (name != nullptr && name[0] != '\0') {
    sb.appendString(name);
  }
  else {
    sb.appendChar('v');
    sb.appendUInt(vd->getId() & Operand::kIdIndexMask);
  }
}

static void X86Context_annotateOperand(X86Context* self,
  StringBuilder& sb, const Operand* op) {

  if (op->isVar()) {
    X86Context_annotateVariable(self, sb, self->_compiler->getVdById(op->getId()));
  }
  else if (op->isMem()) {
    const X86Mem* m = static_cast<const X86Mem*>(op);
    bool isAbsolute = false;

    sb.appendChar('[');
    switch (m->getMemType()) {
      case kMemTypeBaseIndex:
      case kMemTypeStackIndex:
        // [base + index << shift + displacement]
        X86Context_annotateVariable(self, sb, self->_compiler->getVdById(m->getBase()));
        break;

      case kMemTypeLabel:
        // [label + index << shift + displacement]
        sb.appendFormat("L%u", m->getBase());
        break;

      case kMemTypeAbsolute:
        // [absolute]
        isAbsolute = true;
        sb.appendUInt(static_cast<uint32_t>(m->getDisplacement()), 16);
        break;
    }

    if (m->hasIndex()) {
      sb.appendChar('+');
      X86Context_annotateVariable(self, sb, self->_compiler->getVdById(m->getIndex()));

      if (m->getShift()) {
        sb.appendChar('*');
        sb.appendChar("1248"[m->getShift() & 3]);
      }
    }

    if (m->getDisplacement() && !isAbsolute) {
      uint32_t base = 10;
      int32_t dispOffset = m->getDisplacement();

      char prefix = '+';
      if (dispOffset < 0) {
        dispOffset = -dispOffset;
        prefix = '-';
      }

      sb.appendChar(prefix);
      // TODO: Enable again:
      // if ((loggerOptions & (Logger::kOptionHexDisplacement)) != 0 && dispOffset > 9) {
      //   sb.appendString("0x", 2);
      //   base = 16;
      // }
      sb.appendUInt(static_cast<uint32_t>(dispOffset), base);
    }

    sb.appendChar(']');
  }
  else if (op->isImm()) {
    const Imm* i = static_cast<const Imm*>(op);
    int64_t val = i->getInt64();

    /*
    if ((loggerOptions & (1 << Logger::kOptionHexImmediate)) && static_cast<uint64_t>(val) > 9)
      sb.appendUInt(static_cast<uint64_t>(val), 16);
    else*/
      sb.appendInt(val, 10);
  }
  else if (op->isLabel()) {
    sb.appendFormat("L%u", op->getId());
  }
  else {
    sb.appendString("None", 4);
  }
}

static bool X86Context_annotateInstruction(X86Context* self,
  StringBuilder& sb, uint32_t instId, const Operand* opList, uint32_t opCount) {

  sb.appendString(_x86InstInfo[instId].getInstName());
  for (uint32_t i = 0; i < opCount; i++) {
    if (i == 0)
      sb.appendChar(' ');
    else
      sb.appendString(", ", 2);
    X86Context_annotateOperand(self, sb, &opList[i]);
  }
  return true;
}
#endif // !ASMJIT_DISABLE_LOGGER

#if defined(ASMJIT_TRACE)
static void ASMJIT_CDECL X86Context_traceNode(X86Context* self, HLNode* node_, const char* prefix) {
  StringBuilderTmp<256> sb;

  switch (node_->getType()) {
    case HLNode::kTypeAlign: {
      HLAlign* node = static_cast<HLAlign*>(node_);
      sb.appendFormat(".align %u (%s)",
        node->getOffset(),
        node->getAlignMode() == kAlignCode ? "code" : "data");
      break;
    }

    case HLNode::kTypeData: {
      HLData* node = static_cast<HLData*>(node_);
      sb.appendFormat(".embed (%u bytes)", node->getSize());
      break;
    }

    case HLNode::kTypeComment: {
      HLComment* node = static_cast<HLComment*>(node_);
      sb.appendFormat("; %s", node->getComment());
      break;
    }

    case HLNode::kTypeHint: {
      HLHint* node = static_cast<HLHint*>(node_);
      static const char* hint[16] = {
        "alloc",
        "spill",
        "save",
        "save-unuse",
        "unuse"
      };
      sb.appendFormat("[%s] %s",
        hint[node->getHint()], node->getVd()->getName());
      break;
    }

    case HLNode::kTypeLabel: {
      HLLabel* node = static_cast<HLLabel*>(node_);
      sb.appendFormat("L%u: (NumRefs=%u)",
        node->getLabelId(),
        node->getNumRefs());
      break;
    }

    case HLNode::kTypeInst: {
      HLInst* node = static_cast<HLInst*>(node_);
      X86Context_annotateInstruction(self, sb,
        node->getInstId(), node->getOpList(), node->getOpCount());
      break;
    }

    case HLNode::kTypeFunc: {
      HLFunc* node = static_cast<HLFunc*>(node_);
      sb.appendFormat("[func]");
      break;
    }

    case HLNode::kTypeSentinel: {
      HLSentinel* node = static_cast<HLSentinel*>(node_);
      sb.appendFormat("[end]");
      break;
    }

    case HLNode::kTypeRet: {
      HLRet* node = static_cast<HLRet*>(node_);
      sb.appendFormat("[ret]");
      break;
    }

    case HLNode::kTypeCall: {
      HLCall* node = static_cast<HLCall*>(node_);
      sb.appendFormat("[call]");
      break;
    }

    case HLNode::kTypeCallArg: {
      HLCallArg* node = static_cast<HLCallArg*>(node_);
      sb.appendFormat("[sarg]");
      break;
    }

    default: {
      sb.appendFormat("[unknown]");
      break;
    }
  }

  ASMJIT_TLOG("%s[%05u] %s\n", prefix, node_->getFlowId(), sb.getData());
}
#endif // ASMJIT_TRACE

// ============================================================================
// [asmjit::X86Context - Construction / Destruction]
// ============================================================================

X86Context::X86Context(X86Compiler* compiler) : Context(compiler) {
  _varMapToVaListOffset = ASMJIT_OFFSET_OF(X86VarMap, _list);
  _regCount = compiler->_regCount;

  _zsp = compiler->zsp;
  _zbp = compiler->zbp;

  _memSlot._vmem.type = kMemTypeStackIndex;
  _memSlot.setGpdBase(compiler->getArch() == kArchX86);

#if defined(ASMJIT_TRACE)
  _traceNode = (TraceNodeFunc)X86Context_traceNode;
#endif // ASMJIT_TRACE

#if !defined(ASMJIT_DISABLE_LOGGER)
  _emitComments = compiler->getAssembler()->hasLogger();
#endif // !ASMJIT_DISABLE_LOGGER

  _state = &_x86State;
  reset();
}
X86Context::~X86Context() {}

// ============================================================================
// [asmjit::X86Context - Reset]
// ============================================================================

void X86Context::reset(bool releaseMemory) {
  Context::reset(releaseMemory);

  _x86State.reset(0);
  _clobberedRegs.reset();

  _stackFrameCell = nullptr;
  _gaRegs[kX86RegClassGp ] = Utils::bits(_regCount.getGp()) & ~Utils::mask(kX86RegIndexSp);
  _gaRegs[kX86RegClassMm ] = Utils::bits(_regCount.getMm());
  _gaRegs[kX86RegClassK  ] = Utils::bits(_regCount.getK());
  _gaRegs[kX86RegClassXyz] = Utils::bits(_regCount.getXyz());

  _argBaseReg = kInvalidReg; // Used by patcher.
  _varBaseReg = kInvalidReg; // Used by patcher.

  _argBaseOffset = 0;        // Used by patcher.
  _varBaseOffset = 0;        // Used by patcher.

  _argActualDisp = 0;        // Used by translator.
  _varActualDisp = 0;        // Used by translator.
}

// ============================================================================
// [asmjit::X86SpecialInst]
// ============================================================================

struct X86SpecialInst {
  uint8_t inReg;
  uint8_t outReg;
  uint16_t flags;
};

static const X86SpecialInst x86SpecialInstCpuid[] = {
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg },
  { kInvalidReg   , kX86RegIndexBx, kVarAttrWReg },
  { kInvalidReg   , kX86RegIndexCx, kVarAttrWReg },
  { kInvalidReg   , kX86RegIndexDx, kVarAttrWReg }
};

static const X86SpecialInst x86SpecialInstCbwCdqeCwde[] = {
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstCdqCwdCqo[] = {
  { kInvalidReg   , kX86RegIndexDx, kVarAttrWReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstCmpxchg[] = {
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrXReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstCmpxchg8b16b[] = {
  { kX86RegIndexDx, kX86RegIndexDx, kVarAttrXReg },
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg },
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexBx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstDaaDas[] = {
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstDiv[] = {
  { kInvalidReg   , kX86RegIndexDx, kVarAttrXReg },
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstJecxz[] = {
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstLods[] = {
  { kInvalidReg   , kX86RegIndexAx, kVarAttrWReg },
  { kX86RegIndexSi, kX86RegIndexSi, kVarAttrXReg },
  { kX86RegIndexCx, kX86RegIndexCx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstMul[] = {
  { kInvalidReg   , kX86RegIndexDx, kVarAttrWReg },
  { kX86RegIndexAx, kX86RegIndexAx, kVarAttrXReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstMovPtr[] = {
  { kInvalidReg   , kX86RegIndexAx, kVarAttrWReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstMovsCmps[] = {
  { kX86RegIndexDi, kX86RegIndexDi, kVarAttrXReg },
  { kX86RegIndexSi, kX86RegIndexSi, kVarAttrXReg },
  { kX86RegIndexCx, kX86RegIndexCx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstLahf[] = {
  { kInvalidReg   , kX86RegIndexAx, kVarAttrWReg }
};

static const X86SpecialInst x86SpecialInstSahf[] = {
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstMaskmovqMaskmovdqu[] = {
  { kInvalidReg   , kX86RegIndexDi, kVarAttrRReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstRdtscRdtscp[] = {
  { kInvalidReg   , kX86RegIndexDx, kVarAttrWReg },
  { kInvalidReg   , kX86RegIndexAx, kVarAttrWReg },
  { kInvalidReg   , kX86RegIndexCx, kVarAttrWReg }
};

static const X86SpecialInst x86SpecialInstRot[] = {
  { kInvalidReg   , kInvalidReg   , kVarAttrXReg },
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstScas[] = {
  { kX86RegIndexDi, kX86RegIndexDi, kVarAttrXReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexCx, kX86RegIndexCx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstShlrd[] = {
  { kInvalidReg   , kInvalidReg   , kVarAttrXReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstStos[] = {
  { kX86RegIndexDi, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexCx, kX86RegIndexCx, kVarAttrXReg }
};

static const X86SpecialInst x86SpecialInstBlend[] = {
  { kInvalidReg   , kInvalidReg   , kVarAttrWReg },
  { kInvalidReg   , kInvalidReg   , kVarAttrRReg },
  { 0             , kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstXsaveXrstor[] = {
  { kInvalidReg   , kInvalidReg   , 0            },
  { kX86RegIndexDx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg }
};

static const X86SpecialInst x86SpecialInstXgetbv[] = {
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg },
  { kInvalidReg   , kX86RegIndexDx, kVarAttrWReg },
  { kInvalidReg   , kX86RegIndexAx, kVarAttrWReg }
};

static const X86SpecialInst x86SpecialInstXsetbv[] = {
  { kX86RegIndexCx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexDx, kInvalidReg   , kVarAttrRReg },
  { kX86RegIndexAx, kInvalidReg   , kVarAttrRReg }
};

static ASMJIT_INLINE const X86SpecialInst* X86SpecialInst_get(uint32_t instId, const Operand* opList, uint32_t opCount) {
  switch (instId) {
    case kX86InstIdCpuid:
      return x86SpecialInstCpuid;

    case kX86InstIdCbw:
    case kX86InstIdCdqe:
    case kX86InstIdCwde:
      return x86SpecialInstCbwCdqeCwde;

    case kX86InstIdCdq:
    case kX86InstIdCwd:
    case kX86InstIdCqo:
      return x86SpecialInstCdqCwdCqo;

    case kX86InstIdCmpsB:
    case kX86InstIdCmpsD:
    case kX86InstIdCmpsQ:
    case kX86InstIdCmpsW:
    case kX86InstIdRepeCmpsB:
    case kX86InstIdRepeCmpsD:
    case kX86InstIdRepeCmpsQ:
    case kX86InstIdRepeCmpsW:
    case kX86InstIdRepneCmpsB:
    case kX86InstIdRepneCmpsD:
    case kX86InstIdRepneCmpsQ:
    case kX86InstIdRepneCmpsW:
      return x86SpecialInstMovsCmps;

    case kX86InstIdCmpxchg:
      return x86SpecialInstCmpxchg;

    case kX86InstIdCmpxchg8b:
    case kX86InstIdCmpxchg16b:
      return x86SpecialInstCmpxchg8b16b;

    case kX86InstIdDaa:
    case kX86InstIdDas:
      return x86SpecialInstDaaDas;

    case kX86InstIdJecxz:
      return x86SpecialInstJecxz;

    case kX86InstIdIdiv:
    case kX86InstIdDiv:
      return x86SpecialInstDiv;

    case kX86InstIdImul:
      if (opCount == 2)
        return nullptr;
      if (opCount == 3 && !(opList[0].isVar() && opList[1].isVar() && opList[2].isVarOrMem()))
        return nullptr;
      ASMJIT_FALLTHROUGH;

    case kX86InstIdMul:
      return x86SpecialInstMul;

    case kX86InstIdMovPtr:
      return x86SpecialInstMovPtr;

    case kX86InstIdLodsB:
    case kX86InstIdLodsD:
    case kX86InstIdLodsQ:
    case kX86InstIdLodsW:
    case kX86InstIdRepLodsB:
    case kX86InstIdRepLodsD:
    case kX86InstIdRepLodsQ:
    case kX86InstIdRepLodsW:
      return x86SpecialInstLods;

    case kX86InstIdMovsB:
    case kX86InstIdMovsD:
    case kX86InstIdMovsQ:
    case kX86InstIdMovsW:
    case kX86InstIdRepMovsB:
    case kX86InstIdRepMovsD:
    case kX86InstIdRepMovsQ:
    case kX86InstIdRepMovsW:
      return x86SpecialInstMovsCmps;

    case kX86InstIdLahf:
      return x86SpecialInstLahf;

    case kX86InstIdSahf:
      return x86SpecialInstSahf;

    case kX86InstIdMaskmovq:
    case kX86InstIdMaskmovdqu:
      return x86SpecialInstMaskmovqMaskmovdqu;

    // Not supported.
    case kX86InstIdEnter:
    case kX86InstIdLeave:
      return nullptr;

    // Not supported.
    case kX86InstIdRet:
      return nullptr;

    case kX86InstIdMonitor:
    case kX86InstIdMwait:
      // TODO: [COMPILER] Monitor/MWait.
      return nullptr;

    case kX86InstIdPop:
      // TODO: [COMPILER] Pop.
      return nullptr;

    // Not supported.
    case kX86InstIdPopa:
    case kX86InstIdPopf:
      return nullptr;

    case kX86InstIdPush:
      // TODO: [COMPILER] Push.
      return nullptr;

    // Not supported.
    case kX86InstIdPusha:
    case kX86InstIdPushf:
      return nullptr;

    // Rot instruction is special only if the last operand is a variable.
    case kX86InstIdRcl:
    case kX86InstIdRcr:
    case kX86InstIdRol:
    case kX86InstIdRor:
    case kX86InstIdSal:
    case kX86InstIdSar:
    case kX86InstIdShl:
    case kX86InstIdShr:
      if (!opList[1].isVar())
        return nullptr;
      return x86SpecialInstRot;

    // Shld/Shrd instruction is special only if the last operand is a variable.
    case kX86InstIdShld:
    case kX86InstIdShrd:
      if (!opList[2].isVar())
        return nullptr;
      return x86SpecialInstShlrd;

    case kX86InstIdRdtsc:
    case kX86InstIdRdtscp:
      return x86SpecialInstRdtscRdtscp;

    case kX86InstIdScasB:
    case kX86InstIdScasD:
    case kX86InstIdScasQ:
    case kX86InstIdScasW:
    case kX86InstIdRepeScasB:
    case kX86InstIdRepeScasD:
    case kX86InstIdRepeScasQ:
    case kX86InstIdRepeScasW:
    case kX86InstIdRepneScasB:
    case kX86InstIdRepneScasD:
    case kX86InstIdRepneScasQ:
    case kX86InstIdRepneScasW:
      return x86SpecialInstScas;

    case kX86InstIdStosB:
    case kX86InstIdStosD:
    case kX86InstIdStosQ:
    case kX86InstIdStosW:
    case kX86InstIdRepStosB:
    case kX86InstIdRepStosD:
    case kX86InstIdRepStosQ:
    case kX86InstIdRepStosW:
      return x86SpecialInstStos;

    case kX86InstIdBlendvpd:
    case kX86InstIdBlendvps:
    case kX86InstIdPblendvb:
      return x86SpecialInstBlend;

    case kX86InstIdXrstor:
    case kX86InstIdXrstor64:
    case kX86InstIdXsave:
    case kX86InstIdXsave64:
    case kX86InstIdXsaveopt:
    case kX86InstIdXsaveopt64:
      return x86SpecialInstXsaveXrstor;

    case kX86InstIdXgetbv:
      return x86SpecialInstXgetbv;

    case kX86InstIdXsetbv:
      return x86SpecialInstXsetbv;

    default:
      return nullptr;
  }
}

// ============================================================================
// [asmjit::X86Context - EmitLoad]
// ============================================================================

void X86Context::emitLoad(VarData* vd, uint32_t regIndex, const char* reason) {
  ASMJIT_ASSERT(regIndex != kInvalidReg);

  X86Compiler* compiler = getCompiler();
  X86Mem m = getVarMem(vd);

  HLNode* node = nullptr;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      node = compiler->emit(kX86InstIdMov, x86::gpb_lo(regIndex), m);
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      node = compiler->emit(kX86InstIdMov, x86::gpw(regIndex), m);
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kX86InstIdMov, x86::gpd(regIndex), m);
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      ASMJIT_ASSERT(_compiler->getArch() != kArchX86);
      node = compiler->emit(kX86InstIdMov, x86::gpq(regIndex), m);
      break;
#endif // ASMJIT_BUILD_X64

    case kX86VarTypeMm:
      node = compiler->emit(kX86InstIdMovq, x86::mm(regIndex), m);
      break;

    case kX86VarTypeXmm:
      node = compiler->emit(kX86InstIdMovdqa, x86::xmm(regIndex), m);
      break;

    case kX86VarTypeXmmSs:
      node = compiler->emit(kX86InstIdMovss, x86::xmm(regIndex), m);
      break;

    case kX86VarTypeXmmSd:
      node = compiler->emit(kX86InstIdMovsd, x86::xmm(regIndex), m);
      break;

    case kX86VarTypeXmmPs:
      node = compiler->emit(kX86InstIdMovaps, x86::xmm(regIndex), m);
      break;

    case kX86VarTypeXmmPd:
      node = compiler->emit(kX86InstIdMovapd, x86::xmm(regIndex), m);
      break;

    // Compiler doesn't manage FPU stack.
    case kVarTypeFp32:
    case kVarTypeFp64:
    default:
      ASMJIT_NOT_REACHED();
  }

  if (!_emitComments)
    return;
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::X86Context - EmitSave]
// ============================================================================

void X86Context::emitSave(VarData* vd, uint32_t regIndex, const char* reason) {
  ASMJIT_ASSERT(regIndex != kInvalidReg);

  X86Compiler* compiler = getCompiler();
  X86Mem m = getVarMem(vd);

  HLNode* node = nullptr;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      node = compiler->emit(kX86InstIdMov, m, x86::gpb_lo(regIndex));
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      node = compiler->emit(kX86InstIdMov, m, x86::gpw(regIndex));
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kX86InstIdMov, m, x86::gpd(regIndex));
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      node = compiler->emit(kX86InstIdMov, m, x86::gpq(regIndex));
      break;
#endif // ASMJIT_BUILD_X64

    case kX86VarTypeMm:
      node = compiler->emit(kX86InstIdMovq, m, x86::mm(regIndex));
      break;

    case kX86VarTypeXmm:
      node = compiler->emit(kX86InstIdMovdqa, m, x86::xmm(regIndex));
      break;

    case kX86VarTypeXmmSs:
      node = compiler->emit(kX86InstIdMovss, m, x86::xmm(regIndex));
      break;

    case kX86VarTypeXmmSd:
      node = compiler->emit(kX86InstIdMovsd, m, x86::xmm(regIndex));
      break;

    case kX86VarTypeXmmPs:
      node = compiler->emit(kX86InstIdMovaps, m, x86::xmm(regIndex));
      break;

    case kX86VarTypeXmmPd:
      node = compiler->emit(kX86InstIdMovapd, m, x86::xmm(regIndex));
      break;

    // Compiler doesn't manage FPU stack.
    case kVarTypeFp32:
    case kVarTypeFp64:
    default:
      ASMJIT_NOT_REACHED();
  }

  if (!_emitComments)
    return;
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::X86Context - EmitMove]
// ============================================================================

void X86Context::emitMove(VarData* vd, uint32_t toRegIndex, uint32_t fromRegIndex, const char* reason) {
  ASMJIT_ASSERT(toRegIndex != kInvalidReg);
  ASMJIT_ASSERT(fromRegIndex != kInvalidReg);

  X86Compiler* compiler = getCompiler();
  HLNode* node = nullptr;

  switch (vd->getType()) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
    case kVarTypeInt16:
    case kVarTypeUInt16:
    case kVarTypeInt32:
    case kVarTypeUInt32:
      node = compiler->emit(kX86InstIdMov, x86::gpd(toRegIndex), x86::gpd(fromRegIndex));
      break;

#if defined(ASMJIT_BUILD_X64)
    case kVarTypeInt64:
    case kVarTypeUInt64:
      node = compiler->emit(kX86InstIdMov, x86::gpq(toRegIndex), x86::gpq(fromRegIndex));
      break;
#endif // ASMJIT_BUILD_X64

    case kX86VarTypeMm:
      node = compiler->emit(kX86InstIdMovq, x86::mm(toRegIndex), x86::mm(fromRegIndex));
      break;

    case kX86VarTypeXmm:
      node = compiler->emit(kX86InstIdMovaps, x86::xmm(toRegIndex), x86::xmm(fromRegIndex));
      break;

    case kX86VarTypeXmmSs:
      node = compiler->emit(kX86InstIdMovss, x86::xmm(toRegIndex), x86::xmm(fromRegIndex));
      break;

    case kX86VarTypeXmmSd:
      node = compiler->emit(kX86InstIdMovsd, x86::xmm(toRegIndex), x86::xmm(fromRegIndex));
      break;

    case kX86VarTypeXmmPs:
    case kX86VarTypeXmmPd:
      node = compiler->emit(kX86InstIdMovaps, x86::xmm(toRegIndex), x86::xmm(fromRegIndex));
      break;

    case kVarTypeFp32:
    case kVarTypeFp64:
    default:
      // Compiler doesn't manage FPU stack.
      ASMJIT_NOT_REACHED();
  }

  if (!_emitComments)
    return;
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s", reason, vd->getName()));
}

// ============================================================================
// [asmjit::X86Context - EmitSwap]
// ============================================================================

void X86Context::emitSwapGp(VarData* aVd, VarData* bVd, uint32_t aIndex, uint32_t bIndex, const char* reason) {
  ASMJIT_ASSERT(aIndex != kInvalidReg);
  ASMJIT_ASSERT(bIndex != kInvalidReg);

  X86Compiler* compiler = getCompiler();
  HLNode* node = nullptr;

#if defined(ASMJIT_BUILD_X64)
  uint32_t vType = Utils::iMax(aVd->getType(), bVd->getType());
  if (vType == kVarTypeInt64 || vType == kVarTypeUInt64) {
    node = compiler->emit(kX86InstIdXchg, x86::gpq(aIndex), x86::gpq(bIndex));
  }
  else {
#endif // ASMJIT_BUILD_X64
    node = compiler->emit(kX86InstIdXchg, x86::gpd(aIndex), x86::gpd(bIndex));
#if defined(ASMJIT_BUILD_X64)
  }
#endif // ASMJIT_BUILD_X64

  if (!_emitComments)
    return;
  node->setComment(compiler->_stringAllocator.sformat("[%s] %s, %s", reason, aVd->getName(), bVd->getName()));
}

// ============================================================================
// [asmjit::X86Context - EmitPushSequence / EmitPopSequence]
// ============================================================================

void X86Context::emitPushSequence(uint32_t regs) {
  X86Compiler* compiler = getCompiler();
  uint32_t i = 0;

  X86GpReg gpReg(_zsp);
  while (regs != 0) {
    ASMJIT_ASSERT(i < _regCount.getGp());
    if ((regs & 0x1) != 0)
      compiler->emit(kX86InstIdPush, gpReg.setIndex(i));
    i++;
    regs >>= 1;
  }
}

void X86Context::emitPopSequence(uint32_t regs) {
  X86Compiler* compiler = getCompiler();

  if (regs == 0)
    return;

  uint32_t i = static_cast<int32_t>(_regCount.getGp());
  uint32_t mask = 0x1 << static_cast<uint32_t>(i - 1);

  X86GpReg gpReg(_zsp);
  while (i) {
    i--;
    if ((regs & mask) != 0)
      compiler->emit(kX86InstIdPop, gpReg.setIndex(i));
    mask >>= 1;
  }
}

// ============================================================================
// [asmjit::X86Context - EmitConvertVarToVar]
// ============================================================================

void X86Context::emitConvertVarToVar(uint32_t dstType, uint32_t dstIndex, uint32_t srcType, uint32_t srcIndex) {
  X86Compiler* compiler = getCompiler();

  switch (dstType) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
    case kVarTypeInt16:
    case kVarTypeUInt16:
    case kVarTypeInt32:
    case kVarTypeUInt32:
    case kVarTypeInt64:
    case kVarTypeUInt64:
      break;

    case kX86VarTypeXmmPs:
      if (srcType == kX86VarTypeXmmPd || srcType == kX86VarTypeYmmPd) {
        compiler->emit(kX86InstIdCvtpd2ps, x86::xmm(dstIndex), x86::xmm(srcIndex));
        return;
      }
      ASMJIT_FALLTHROUGH;

    case kX86VarTypeXmmSs:
      if (srcType == kX86VarTypeXmmSd || srcType == kX86VarTypeXmmPd || srcType == kX86VarTypeYmmPd) {
        compiler->emit(kX86InstIdCvtsd2ss, x86::xmm(dstIndex), x86::xmm(srcIndex));
        return;
      }

      if (Utils::inInterval<uint32_t>(srcType, _kVarTypeIntStart, _kVarTypeIntEnd)) {
        // TODO: [COMPILER] Variable conversion not supported.
        ASMJIT_NOT_REACHED();
      }
      break;

    case kX86VarTypeXmmPd:
      if (srcType == kX86VarTypeXmmPs || srcType == kX86VarTypeYmmPs) {
        compiler->emit(kX86InstIdCvtps2pd, x86::xmm(dstIndex), x86::xmm(srcIndex));
        return;
      }
      ASMJIT_FALLTHROUGH;

    case kX86VarTypeXmmSd:
      if (srcType == kX86VarTypeXmmSs || srcType == kX86VarTypeXmmPs || srcType == kX86VarTypeYmmPs) {
        compiler->emit(kX86InstIdCvtss2sd, x86::xmm(dstIndex), x86::xmm(srcIndex));
        return;
      }

      if (Utils::inInterval<uint32_t>(srcType, _kVarTypeIntStart, _kVarTypeIntEnd)) {
        // TODO: [COMPILER] Variable conversion not supported.
        ASMJIT_NOT_REACHED();
      }
      break;
  }
}

// ============================================================================
// [asmjit::X86Context - EmitMoveVarOnStack / EmitMoveImmOnStack]
// ============================================================================

void X86Context::emitMoveVarOnStack(
  uint32_t dstType, const X86Mem* dst,
  uint32_t srcType, uint32_t srcIndex) {

  ASMJIT_ASSERT(srcIndex != kInvalidReg);
  X86Compiler* compiler = getCompiler();

  X86Mem m0(*dst);
  X86Reg r0, r1;

  uint32_t regSize = compiler->getRegSize();
  uint32_t instId;

  switch (dstType) {
    case kVarTypeInt8:
    case kVarTypeUInt8:
      // Move DWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeMm, kX86VarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeXmm, kX86VarTypeXmmPd))
        goto _MovXmmD;
      break;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      // Extend BYTE->WORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kX86RegTypeGpbLo, srcIndex);

        instId = (dstType == kVarTypeInt16 && srcType == kVarTypeInt8) ? kX86InstIdMovsx : kX86InstIdMovzx;
        goto _ExtendMovGpD;
      }

      // Move DWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeMm, kX86VarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeXmm, kX86VarTypeXmmPd))
        goto _MovXmmD;
      break;

    case kVarTypeInt32:
    case kVarTypeUInt32:
      // Extend BYTE->DWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kX86RegTypeGpbLo, srcIndex);

        instId = (dstType == kVarTypeInt32 && srcType == kVarTypeInt8) ? kX86InstIdMovsx : kX86InstIdMovzx;
        goto _ExtendMovGpD;
      }

      // Extend WORD->DWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kX86RegTypeGpw, srcIndex);

        instId = (dstType == kVarTypeInt32 && srcType == kVarTypeInt16) ? kX86InstIdMovsx : kX86InstIdMovzx;
        goto _ExtendMovGpD;
      }

      // Move DWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt64))
        goto _MovGpD;

      // Move DWORD (Mm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeMm, kX86VarTypeMm))
        goto _MovMmD;

      // Move DWORD (Xmm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeXmm, kX86VarTypeXmmPd))
        goto _MovXmmD;
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
      // Extend BYTE->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kX86RegTypeGpbLo, srcIndex);

        instId = (dstType == kVarTypeInt64 && srcType == kVarTypeInt8) ? kX86InstIdMovsx : kX86InstIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kX86RegTypeGpw, srcIndex);

        instId = (dstType == kVarTypeInt64 && srcType == kVarTypeInt16) ? kX86InstIdMovsx : kX86InstIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt32)) {
        r1.setSize(4);
        r1.setCode(kX86RegTypeGpd, srcIndex);

        instId = kX86InstIdMovsxd;
        if (dstType == kVarTypeInt64 && srcType == kVarTypeInt32)
          goto _ExtendMovGpXQ;
        else
          goto _ZeroExtendGpDQ;
      }

      // Move QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt64, kVarTypeUInt64))
        goto _MovGpQ;

      // Move QWORD (Mm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeMm, kX86VarTypeMm))
        goto _MovMmQ;

      // Move QWORD (Xmm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeXmm, kX86VarTypeXmmPd))
        goto _MovXmmQ;
      break;

    case kX86VarTypeMm:
      // Extend BYTE->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt8, kVarTypeUInt8)) {
        r1.setSize(1);
        r1.setCode(kX86RegTypeGpbLo, srcIndex);

        instId = kX86InstIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt16, kVarTypeUInt16)) {
        r1.setSize(2);
        r1.setCode(kX86RegTypeGpw, srcIndex);

        instId = kX86InstIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt32, kVarTypeUInt32))
        goto _ExtendMovGpDQ;

      // Move QWORD (Gp).
      if (Utils::inInterval<uint32_t>(srcType, kVarTypeInt64, kVarTypeUInt64))
        goto _MovGpQ;

      // Move QWORD (Mm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeMm, kX86VarTypeMm))
        goto _MovMmQ;

      // Move QWORD (Xmm).
      if (Utils::inInterval<uint32_t>(srcType, kX86VarTypeXmm, kX86VarTypeXmmPd))
        goto _MovXmmQ;
      break;

    case kVarTypeFp32:
    case kX86VarTypeXmmSs:
      // Move FLOAT.
      if (srcType == kX86VarTypeXmmSs || srcType == kX86VarTypeXmmPs || srcType == kX86VarTypeXmm)
        goto _MovXmmD;

      ASMJIT_NOT_REACHED();
      break;

    case kVarTypeFp64:
    case kX86VarTypeXmmSd:
      // Move DOUBLE.
      if (srcType == kX86VarTypeXmmSd || srcType == kX86VarTypeXmmPd || srcType == kX86VarTypeXmm)
        goto _MovXmmQ;

      ASMJIT_NOT_REACHED();
      break;

    case kX86VarTypeXmm:
      // TODO: [COMPILER].
      ASMJIT_NOT_REACHED();
      break;

    case kX86VarTypeXmmPs:
      // TODO: [COMPILER].
      ASMJIT_NOT_REACHED();
      break;

    case kX86VarTypeXmmPd:
      // TODO: [COMPILER].
      ASMJIT_NOT_REACHED();
      break;
  }
  return;

  // Extend+Move Gp.
_ExtendMovGpD:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kX86RegTypeGpd, srcIndex);

  compiler->emit(instId, r0, r1);
  compiler->emit(kX86InstIdMov, m0, r0);
  return;

_ExtendMovGpXQ:
  if (regSize == 8) {
    m0.setSize(8);
    r0.setSize(8);
    r0.setCode(kX86RegTypeGpq, srcIndex);

    compiler->emit(instId, r0, r1);
    compiler->emit(kX86InstIdMov, m0, r0);
  }
  else {
    m0.setSize(4);
    r0.setSize(4);
    r0.setCode(kX86RegTypeGpd, srcIndex);

    compiler->emit(instId, r0, r1);

_ExtendMovGpDQ:
    compiler->emit(kX86InstIdMov, m0, r0);
    m0.adjust(4);
    compiler->emit(kX86InstIdAnd, m0, 0);
  }
  return;

_ZeroExtendGpDQ:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kX86RegTypeGpd, srcIndex);
  goto _ExtendMovGpDQ;

  // Move Gp.
_MovGpD:
  m0.setSize(4);
  r0.setSize(4);
  r0.setCode(kX86RegTypeGpd, srcIndex);
  compiler->emit(kX86InstIdMov, m0, r0);
  return;

_MovGpQ:
  m0.setSize(8);
  r0.setSize(8);
  r0.setCode(kX86RegTypeGpq, srcIndex);
  compiler->emit(kX86InstIdMov, m0, r0);
  return;

  // Move Mm.
_MovMmD:
  m0.setSize(4);
  r0.setSize(8);
  r0.setCode(kX86RegTypeMm, srcIndex);
  compiler->emit(kX86InstIdMovd, m0, r0);
  return;

_MovMmQ:
  m0.setSize(8);
  r0.setSize(8);
  r0.setCode(kX86RegTypeMm, srcIndex);
  compiler->emit(kX86InstIdMovq, m0, r0);
  return;

  // Move Xmm.
_MovXmmD:
  m0.setSize(4);
  r0.setSize(16);
  r0.setCode(kX86RegTypeXmm, srcIndex);
  compiler->emit(kX86InstIdMovss, m0, r0);
  return;

_MovXmmQ:
  m0.setSize(8);
  r0.setSize(16);
  r0.setCode(kX86RegTypeXmm, srcIndex);
  compiler->emit(kX86InstIdMovlps, m0, r0);
}

void X86Context::emitMoveImmOnStack(uint32_t dstType, const X86Mem* dst, const Imm* src) {
  X86Compiler* compiler = getCompiler();

  X86Mem mem(*dst);
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
      goto _Move32;

    case kVarTypeInt16:
    case kVarTypeUInt16:
      imm.truncateTo16Bits();
      goto _Move32;

    case kVarTypeInt32:
    case kVarTypeUInt32:
_Move32:
      imm.truncateTo32Bits();
      compiler->emit(kX86InstIdMov, mem, imm);
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
_Move64:
      if (regSize == 4) {
        uint32_t hi = imm.getUInt32Hi();

        // Lo-Part.
        compiler->emit(kX86InstIdMov, mem, imm.truncateTo32Bits());
        mem.adjust(regSize);

        // Hi-Part.
        compiler->emit(kX86InstIdMov, mem, imm.setUInt32(hi));
      }
      else {
        compiler->emit(kX86InstIdMov, mem, imm);
      }
      break;

    case kVarTypeFp32:
      goto _Move32;

    case kVarTypeFp64:
      goto _Move64;

    case kX86VarTypeMm:
      goto _Move64;

    case kX86VarTypeXmm:
    case kX86VarTypeXmmSs:
    case kX86VarTypeXmmPs:
    case kX86VarTypeXmmSd:
    case kX86VarTypeXmmPd:
      if (regSize == 4) {
        uint32_t hi = imm.getUInt32Hi();

        // Lo part.
        compiler->emit(kX86InstIdMov, mem, imm.truncateTo32Bits());
        mem.adjust(regSize);

        // Hi part.
        compiler->emit(kX86InstIdMov, mem, imm.setUInt32(hi));
        mem.adjust(regSize);

        // Zero part.
        compiler->emit(kX86InstIdMov, mem, imm.setUInt32(0));
        mem.adjust(regSize);

        compiler->emit(kX86InstIdMov, mem, imm);
      }
      else {
        // Lo/Hi parts.
        compiler->emit(kX86InstIdMov, mem, imm);
        mem.adjust(regSize);

        // Zero part.
        compiler->emit(kX86InstIdMov, mem, imm.setUInt32(0));
      }
      break;

    default:
      ASMJIT_NOT_REACHED();
      break;
  }
}

// ============================================================================
// [asmjit::X86Context - EmitMoveImmToReg]
// ============================================================================

void X86Context::emitMoveImmToReg(uint32_t dstType, uint32_t dstIndex, const Imm* src) {
  ASMJIT_ASSERT(dstIndex != kInvalidReg);
  X86Compiler* compiler = getCompiler();

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
      r0.setCode(kX86RegTypeGpd, dstIndex);
      compiler->emit(kX86InstIdMov, r0, imm);
      break;

    case kVarTypeInt64:
    case kVarTypeUInt64:
      // Move to Gpd register will also clear high DWORD of Gpq register in
      // 64-bit mode.
      if (imm.isUInt32())
        goto _Move32Truncate;

      r0.setSize(8);
      r0.setCode(kX86RegTypeGpq, dstIndex);
      compiler->emit(kX86InstIdMov, r0, imm);
      break;

    case kVarTypeFp32:
    case kVarTypeFp64:
      // Compiler doesn't manage FPU stack.
      ASMJIT_NOT_REACHED();
      break;

    case kX86VarTypeMm:
      // TODO: [COMPILER] EmitMoveImmToReg.
      break;

    case kX86VarTypeXmm:
    case kX86VarTypeXmmSs:
    case kX86VarTypeXmmSd:
    case kX86VarTypeXmmPs:
    case kX86VarTypeXmmPd:
      // TODO: [COMPILER] EmitMoveImmToReg.
      break;

    default:
      ASMJIT_NOT_REACHED();
      break;
  }
}

// ============================================================================
// [asmjit::X86Context - Register Management]
// ============================================================================

#if defined(ASMJIT_DEBUG)
template<int C>
static ASMJIT_INLINE void X86Context_checkStateVars(X86Context* self) {
  X86VarState* state = self->getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t regIndex;
  uint32_t regMask;
  uint32_t regCount = self->_regCount.get(C);

  uint32_t occupied = state->_occupied.get(C);
  uint32_t modified = state->_modified.get(C);

  for (regIndex = 0, regMask = 1; regIndex < regCount; regIndex++, regMask <<= 1) {
    VarData* vd = sVars[regIndex];

    if (vd == nullptr) {
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

void X86Context::_checkState() {
  X86Context_checkStateVars<kX86RegClassGp >(this);
  X86Context_checkStateVars<kX86RegClassMm >(this);
  X86Context_checkStateVars<kX86RegClassXyz>(this);
}
#else
void X86Context::_checkState() {}
#endif // ASMJIT_DEBUG

// ============================================================================
// [asmjit::X86Context - State - Load]
// ============================================================================

template<int C>
static ASMJIT_INLINE void X86Context_loadStateVars(X86Context* self, X86VarState* src) {
  X86VarState* cur = self->getState();

  VarData** cVars = cur->getListByClass(C);
  VarData** sVars = src->getListByClass(C);

  uint32_t regIndex;
  uint32_t modified = src->_modified.get(C);
  uint32_t regCount = self->_regCount.get(C);

  for (regIndex = 0; regIndex < regCount; regIndex++, modified >>= 1) {
    VarData* vd = sVars[regIndex];
    cVars[regIndex] = vd;

    if (vd == nullptr)
      continue;

    vd->setState(kVarStateReg);
    vd->setRegIndex(regIndex);
    vd->setModified(modified & 0x1);
  }
}

void X86Context::loadState(VarState* src_) {
  X86VarState* cur = getState();
  X86VarState* src = static_cast<X86VarState*>(src_);

  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  // Load allocated variables.
  X86Context_loadStateVars<kX86RegClassGp >(this, src);
  X86Context_loadStateVars<kX86RegClassMm >(this, src);
  X86Context_loadStateVars<kX86RegClassXyz>(this, src);

  // Load masks.
  cur->_occupied = src->_occupied;
  cur->_modified = src->_modified;

  // Load states of other variables and clear their 'Modified' flags.
  for (uint32_t i = 0; i < vdCount; i++) {
    uint32_t vState = src->_cells[i].getState();

    if (vState == kVarStateReg)
      continue;

    vdArray[i]->setState(vState);
    vdArray[i]->setRegIndex(kInvalidReg);
    vdArray[i]->setModified(false);
  }

  ASMJIT_X86_CHECK_STATE
}

// ============================================================================
// [asmjit::X86Context - State - Save]
// ============================================================================

VarState* X86Context::saveState() {
  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  size_t size = Utils::alignTo<size_t>(
    sizeof(X86VarState) + vdCount * sizeof(X86StateCell), sizeof(void*));

  X86VarState* cur = getState();
  X86VarState* dst = _zoneAllocator.allocT<X86VarState>(size);

  if (dst == nullptr)
    return nullptr;

  // Store links.
  ::memcpy(dst->_list, cur->_list, X86VarState::kAllCount * sizeof(VarData*));

  // Store masks.
  dst->_occupied = cur->_occupied;
  dst->_modified = cur->_modified;

  // Store cells.
  for (uint32_t i = 0; i < vdCount; i++) {
    VarData* vd = static_cast<VarData*>(vdArray[i]);
    X86StateCell& cell = dst->_cells[i];

    cell.reset();
    cell.setState(vd->getState());
  }

  return dst;
}

// ============================================================================
// [asmjit::X86Context - State - Switch]
// ============================================================================

template<int C>
static ASMJIT_INLINE void X86Context_switchStateVars(X86Context* self, X86VarState* src) {
  X86VarState* dst = self->getState();

  VarData** dVars = dst->getListByClass(C);
  VarData** sVars = src->getListByClass(C);

  X86StateCell* cells = src->_cells;
  uint32_t regCount = self->_regCount.get(C);
  bool didWork;

  do {
    didWork = false;

    for (uint32_t regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* dVd = dVars[regIndex];
      VarData* sVd = sVars[regIndex];

      if (dVd == sVd)
        continue;

      if (dVd != nullptr) {
        const X86StateCell& cell = cells[dVd->getLocalId()];

        if (cell.getState() != kVarStateReg) {
          if (cell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);

          dVd = nullptr;
          didWork = true;

          if (sVd == nullptr)
            continue;
        }
      }

      if (dVd == nullptr && sVd != nullptr) {
_MoveOrLoad:
        if (sVd->getRegIndex() != kInvalidReg)
          self->move<C>(sVd, regIndex);
        else
          self->load<C>(sVd, regIndex);

        didWork = true;
        continue;
      }

      if (dVd != nullptr) {
        const X86StateCell& cell = cells[dVd->getLocalId()];
        if (sVd == nullptr) {
          if (cell.getState() == kVarStateReg)
            continue;

          if (cell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);

          didWork = true;
          continue;
        }
        else {
          if (cell.getState() == kVarStateReg) {
            if (dVd->getRegIndex() != kInvalidReg && sVd->getRegIndex() != kInvalidReg) {
              if (C == kX86RegClassGp) {
                self->swapGp(dVd, sVd);
              }
              else {
                self->spill<C>(dVd);
                self->move<C>(sVd, regIndex);
              }

              didWork = true;
              continue;
            }
            else {
              didWork = true;
              continue;
            }
          }

          if (cell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);
          goto _MoveOrLoad;
        }
      }
    }
  } while (didWork);

  uint32_t dModified = dst->_modified.get(C);
  uint32_t sModified = src->_modified.get(C);

  if (dModified != sModified) {
    for (uint32_t regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* vd = dVars[regIndex];

      if (vd == nullptr)
        continue;

      if ((dModified & regMask) && !(sModified & regMask)) {
        self->save<C>(vd);
        continue;
      }

      if (!(dModified & regMask) && (sModified & regMask)) {
        self->modify<C>(vd);
        continue;
      }
    }
  }
}

void X86Context::switchState(VarState* src_) {
  ASMJIT_ASSERT(src_ != nullptr);

  X86VarState* cur = getState();
  X86VarState* src = static_cast<X86VarState*>(src_);

  // Ignore if both states are equal.
  if (cur == src)
    return;

  // Switch variables.
  X86Context_switchStateVars<kX86RegClassGp >(this, src);
  X86Context_switchStateVars<kX86RegClassMm >(this, src);
  X86Context_switchStateVars<kX86RegClassXyz>(this, src);

  // Calculate changed state.
  VarData** vdArray = _contextVd.getData();
  uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());

  X86StateCell* cells = src->_cells;
  for (uint32_t i = 0; i < vdCount; i++) {
    VarData* vd = static_cast<VarData*>(vdArray[i]);
    const X86StateCell& cell = cells[i];
    uint32_t vState = cell.getState();

    if (vState != kVarStateReg) {
      vd->setState(vState);
      vd->setModified(false);
    }
  }

  ASMJIT_X86_CHECK_STATE
}

// ============================================================================
// [asmjit::X86Context - State - Intersect]
// ============================================================================

// The algorithm is actually not so smart, but tries to find an intersection od
// `a` and `b` and tries to move/alloc a variable into that location if it's
// possible. It also finds out which variables will be spilled/unused  by `a`
// and `b` and performs that action here. It may improve the switch state code
// in certain cases, but doesn't necessarily do the best job possible.
template<int C>
static ASMJIT_INLINE void X86Context_intersectStateVars(X86Context* self, X86VarState* a, X86VarState* b) {
  X86VarState* dst = self->getState();

  VarData** dVars = dst->getListByClass(C);
  VarData** aVars = a->getListByClass(C);
  VarData** bVars = b->getListByClass(C);

  X86StateCell* aCells = a->_cells;
  X86StateCell* bCells = b->_cells;

  uint32_t regCount = self->_regCount.get(C);
  bool didWork;

  // Similar to `switchStateVars()`, we iterate over and over until there is
  // no work to be done.
  do {
    didWork = false;

    for (uint32_t regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* dVd = dVars[regIndex];

      VarData* aVd = aVars[regIndex];
      VarData* bVd = bVars[regIndex];

      if (dVd == aVd)
        continue;

      if (dVd != nullptr) {
        const X86StateCell& aCell = aCells[dVd->getLocalId()];
        const X86StateCell& bCell = bCells[dVd->getLocalId()];

        if (aCell.getState() != kVarStateReg && bCell.getState() != kVarStateReg) {
          if (aCell.getState() == kVarStateMem || bCell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);

          dVd = nullptr;
          didWork = true;

          if (aVd == nullptr)
            continue;
        }
      }

      if (dVd == nullptr && aVd != nullptr) {
        if (aVd->getRegIndex() != kInvalidReg)
          self->move<C>(aVd, regIndex);
        else
          self->load<C>(aVd, regIndex);

        didWork = true;
        continue;
      }

      if (dVd != nullptr) {
        const X86StateCell& aCell = aCells[dVd->getLocalId()];
        const X86StateCell& bCell = bCells[dVd->getLocalId()];

        if (aVd == nullptr) {
          if (aCell.getState() == kVarStateReg || bCell.getState() == kVarStateReg)
            continue;

          if (aCell.getState() == kVarStateMem || bCell.getState() == kVarStateMem)
            self->spill<C>(dVd);
          else
            self->unuse<C>(dVd);

          didWork = true;
          continue;
        }
        else if (C == kX86RegClassGp) {
          if (aCell.getState() == kVarStateReg) {
            if (dVd->getRegIndex() != kInvalidReg && aVd->getRegIndex() != kInvalidReg) {
              self->swapGp(dVd, aVd);

              didWork = true;
              continue;
            }
          }
        }
      }
    }
  } while (didWork);

  uint32_t dModified = dst->_modified.get(C);
  uint32_t aModified = a->_modified.get(C);

  if (dModified != aModified) {
    for (uint32_t regIndex = 0, regMask = 0x1; regIndex < regCount; regIndex++, regMask <<= 1) {
      VarData* vd = dVars[regIndex];

      if (vd == nullptr)
        continue;

      const X86StateCell& aCell = aCells[vd->getLocalId()];
      if ((dModified & regMask) && !(aModified & regMask) && aCell.getState() == kVarStateReg)
        self->save<C>(vd);
    }
  }
}

void X86Context::intersectStates(VarState* a_, VarState* b_) {
  X86VarState* a = static_cast<X86VarState*>(a_);
  X86VarState* b = static_cast<X86VarState*>(b_);

  ASMJIT_ASSERT(a != nullptr);
  ASMJIT_ASSERT(b != nullptr);

  X86Context_intersectStateVars<kX86RegClassGp >(this, a, b);
  X86Context_intersectStateVars<kX86RegClassMm >(this, a, b);
  X86Context_intersectStateVars<kX86RegClassXyz>(this, a, b);

  ASMJIT_X86_CHECK_STATE
}

// ============================================================================
// [asmjit::X86Context - GetJccFlow / GetOppositeJccFlow]
// ============================================================================

//! \internal
static ASMJIT_INLINE HLNode* X86Context_getJccFlow(HLJump* jNode) {
  if (jNode->isTaken())
    return jNode->getTarget();
  else
    return jNode->getNext();
}

//! \internal
static ASMJIT_INLINE HLNode* X86Context_getOppositeJccFlow(HLJump* jNode) {
  if (jNode->isTaken())
    return jNode->getNext();
  else
    return jNode->getTarget();
}

// ============================================================================
// [asmjit::X86Context - SingleVarInst]
// ============================================================================

//! \internal
static void X86Context_prepareSingleVarInst(uint32_t instId, VarAttr* va) {
  switch (instId) {
    // - andn     reg, reg ; Set all bits in reg to 0.
    // - xor/pxor reg, reg ; Set all bits in reg to 0.
    // - sub/psub reg, reg ; Set all bits in reg to 0.
    // - pcmpgt   reg, reg ; Set all bits in reg to 0.
    // - pcmpeq   reg, reg ; Set all bits in reg to 1.
    case kX86InstIdPandn     :
    case kX86InstIdXor       : case kX86InstIdXorpd     : case kX86InstIdXorps     : case kX86InstIdPxor      :
    case kX86InstIdSub:
    case kX86InstIdPsubb     : case kX86InstIdPsubw     : case kX86InstIdPsubd     : case kX86InstIdPsubq     :
    case kX86InstIdPsubsb    : case kX86InstIdPsubsw    : case kX86InstIdPsubusb   : case kX86InstIdPsubusw   :
    case kX86InstIdPcmpeqb   : case kX86InstIdPcmpeqw   : case kX86InstIdPcmpeqd   : case kX86InstIdPcmpeqq   :
    case kX86InstIdPcmpgtb   : case kX86InstIdPcmpgtw   : case kX86InstIdPcmpgtd   : case kX86InstIdPcmpgtq   :
      va->andNotFlags(kVarAttrRReg);
      break;

    // - and      reg, reg ; Nop.
    // - or       reg, reg ; Nop.
    // - xchg     reg, reg ; Nop.
    case kX86InstIdAnd       : case kX86InstIdAndpd     : case kX86InstIdAndps     : case kX86InstIdPand      :
    case kX86InstIdOr        : case kX86InstIdOrpd      : case kX86InstIdOrps      : case kX86InstIdPor       :
    case kX86InstIdXchg      :
      va->andNotFlags(kVarAttrWReg);
      break;
  }
}

// ============================================================================
// [asmjit::X86Context - Helpers]
// ============================================================================

//! \internal
//!
//! Get mask of all registers actually used to pass function arguments.
static ASMJIT_INLINE X86RegMask X86Context_getUsedArgs(X86Context* self, X86CallNode* node, X86FuncDecl* decl) {
  X86RegMask regs;
  regs.reset();

  uint32_t i;
  uint32_t argCount = decl->getNumArgs();

  for (i = 0; i < argCount; i++) {
    const FuncInOut& arg = decl->getArg(i);
    if (!arg.hasRegIndex())
      continue;
    regs.or_(x86VarTypeToClass(arg.getVarType()), Utils::mask(arg.getRegIndex()));
  }

  return regs;
}

// ============================================================================
// [asmjit::X86Context - SArg Insertion]
// ============================================================================

struct SArgData {
  VarData* sVd;
  VarData* cVd;
  HLCallArg* sArg;
  uint32_t aType;
};

#define SARG(dst, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24) \
  (s0  <<  0) | (s1  <<  1) | (s2  <<  2) | (s3  <<  3) | (s4  <<  4) | (s5  <<  5) | (s6  <<  6) | (s7  <<  7) | \
  (s8  <<  8) | (s9  <<  9) | (s10 << 10) | (s11 << 11) | (s12 << 12) | (s13 << 13) | (s14 << 14) | (s15 << 15) | \
  (s16 << 16) | (s17 << 17) | (s18 << 18) | (s19 << 19) | (s20 << 20) | (s21 << 21) | (s22 << 22) | (s23 << 23) | \
  (s24 << 24)
#define A 0 // Auto-convert (doesn't need conversion step).
static const uint32_t X86Context_sArgConvTable[kX86VarTypeCount] = {
  // dst <- | i8| u8|i16|u16|i32|u32|i64|u64| iP| uP|f32|f64|mmx| k |xmm|xSs|xPs|xSd|xPd|ymm|yPs|yPd|zmm|zPs|zPd|
  //--------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
  SARG(i8   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(u8   , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(i16  , A , A , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(u16  , A , A , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(i32  , A , A , A , A , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(u32  , A , A , A , A , 0 , 0 , 0 , 0 , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(i64  , A , A , A , A , A , A , 0 , 0 , A , A , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(u64  , A , A , A , A , A , A , 0 , 0 , A , A , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(iPtr , A , A , A , A , A , A , A , A , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(uPtr , A , A , A , A , A , A , A , A , 0 , 0 , A , A , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 0 , 1 , 1 , 0 , 1 , 1 ),
  SARG(f32  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , A , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1 ),
  SARG(f64  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , A , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 0 ),
  SARG(mmx  , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ),
  SARG(k    , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ),
  SARG(xmm  , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ),
  SARG(xSs  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1 ),
  SARG(xPs  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1 ),
  SARG(xSd  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 0 ),
  SARG(xPd  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 0 ),
  SARG(ymm  , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ),
  SARG(yPs  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1 ),
  SARG(yPd  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 0 ),
  SARG(zmm  , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ),
  SARG(zPs  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1 ),
  SARG(zPd  , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 0 )
};
#undef A
#undef SARG

static ASMJIT_INLINE bool X86Context_mustConvertSArg(X86Context* self, uint32_t aType, uint32_t sType) {
  return (X86Context_sArgConvTable[aType] & (1 << sType)) != 0;
}

static ASMJIT_INLINE uint32_t X86Context_typeOfConvertedSArg(X86Context* self, uint32_t aType, uint32_t sType) {
  ASMJIT_ASSERT(X86Context_mustConvertSArg(self, aType, sType));

  if (Utils::inInterval<uint32_t>(aType, _kVarTypeIntStart, _kVarTypeIntEnd))
    return aType;

  if (aType == kVarTypeFp32) return kX86VarTypeXmmSs;
  if (aType == kVarTypeFp64) return kX86VarTypeXmmSd;

  return aType;
}

static ASMJIT_INLINE Error X86Context_insertHLCallArg(
  X86Context* self, X86CallNode* call,
  VarData* sVd, const uint32_t* gaRegs,
  const FuncInOut& arg, uint32_t argIndex,
  SArgData* sArgList, uint32_t& sArgCount) {

  X86Compiler* compiler = self->getCompiler();
  uint32_t i;

  uint32_t aType = arg.getVarType();
  uint32_t sType = sVd->getType();

  // First locate or create sArgBase.
  for (i = 0; i < sArgCount; i++) {
    if (sArgList[i].sVd == sVd && sArgList[i].cVd == nullptr)
      break;
  }

  SArgData* sArgData = &sArgList[i];

  if (i == sArgCount) {
    sArgData->sVd = sVd;
    sArgData->cVd = nullptr;
    sArgData->sArg = nullptr;
    sArgData->aType = 0xFF;
    sArgCount++;
  }

  const VarInfo& sInfo = _x86VarInfo[sType];
  uint32_t sClass = sInfo.getRegClass();

  if (X86Context_mustConvertSArg(self, aType, sType)) {
    uint32_t cType = X86Context_typeOfConvertedSArg(self, aType, sType);

    const VarInfo& cInfo = _x86VarInfo[cType];
    uint32_t cClass = cInfo.getRegClass();

    while (++i < sArgCount) {
      sArgData = &sArgList[i];
      if (sArgData->sVd != sVd)
        break;

      if (sArgData->cVd->getType() != cType || sArgData->aType != aType)
        continue;

      sArgData->sArg->_args |= Utils::mask(argIndex);
      return kErrorOk;
    }

    VarData* cVd = compiler->_newVd(cInfo, nullptr);
    if (cVd == nullptr)
      return kErrorNoHeapMemory;

    HLCallArg* sArg = compiler->newNode<HLCallArg>(call, sVd, cVd);
    if (sArg == nullptr)
      return kErrorNoHeapMemory;

    X86VarMap* map = self->newVarMap(2);
    if (map == nullptr)
      return kErrorNoHeapMemory;

    ASMJIT_PROPAGATE_ERROR(self->_registerContextVar(cVd));
    ASMJIT_PROPAGATE_ERROR(self->_registerContextVar(sVd));

    map->_vaCount = 2;
    map->_count.reset();
    map->_count.add(sClass);
    map->_count.add(cClass);

    map->_start.reset();
    map->_inRegs.reset();
    map->_outRegs.reset();
    map->_clobberedRegs.reset();

    if (sClass <= cClass) {
      map->_list[0].setup(sVd, kVarAttrRReg, 0, gaRegs[sClass]);
      map->_list[1].setup(cVd, kVarAttrWReg, 0, gaRegs[cClass]);
      map->_start.set(cClass, sClass != cClass);
    }
    else {
      map->_list[0].setup(cVd, kVarAttrWReg, 0, gaRegs[cClass]);
      map->_list[1].setup(sVd, kVarAttrRReg, 0, gaRegs[sClass]);
      map->_start.set(sClass, 1);
    }

    sArg->setMap(map);
    sArg->_args |= Utils::mask(argIndex);

    compiler->addNodeBefore(sArg, call);
    ::memmove(sArgData + 1, sArgData, (sArgCount - i) * sizeof(SArgData));

    sArgData->sVd = sVd;
    sArgData->cVd = cVd;
    sArgData->sArg = sArg;
    sArgData->aType = aType;

    sArgCount++;
    return kErrorOk;
  }
  else {
    HLCallArg* sArg = sArgData->sArg;
    ASMJIT_PROPAGATE_ERROR(self->_registerContextVar(sVd));

    if (sArg == nullptr) {
      sArg = compiler->newNode<HLCallArg>(call, sVd, (VarData*)nullptr);
      if (sArg == nullptr)
        return kErrorNoHeapMemory;

      X86VarMap* map = self->newVarMap(1);
      if (map == nullptr)
        return kErrorNoHeapMemory;

      map->_vaCount = 1;
      map->_count.reset();
      map->_count.add(sClass);
      map->_start.reset();
      map->_inRegs.reset();
      map->_outRegs.reset();
      map->_clobberedRegs.reset();
      map->_list[0].setup(sVd, kVarAttrRReg, 0, gaRegs[sClass]);

      sArg->setMap(map);
      sArgData->sArg = sArg;

      compiler->addNodeBefore(sArg, call);
    }

    sArg->_args |= Utils::mask(argIndex);
    return kErrorOk;
  }
}

// ============================================================================
// [asmjit::X86Context - Fetch]
// ============================================================================

//! \internal
//!
//! Prepare the given function `func`.
//!
//! For each node:
//! - Create and assign groupId and flowId.
//! - Collect all variables and merge them to vaList.
Error X86Context::fetch() {
  ASMJIT_TLOG("[F] ======= Fetch (Begin)\n");

  X86Compiler* compiler = getCompiler();
  X86FuncNode* func = getFunc();

  uint32_t arch = compiler->getArch();

  HLNode* node_ = func;
  HLNode* next = nullptr;
  HLNode* stop = getStop();

  uint32_t flowId = 0;

  VarAttr vaTmpList[80];
  SArgData sArgList[80];

  PodList<HLNode*>::Link* jLink = nullptr;

  // Function flags.
  func->clearFuncFlags(
    kFuncFlagIsNaked    |
    kFuncFlagX86Emms    |
    kFuncFlagX86SFence  |
    kFuncFlagX86LFence  );

  if (func->getHint(kFuncHintNaked    ) != 0) func->addFuncFlags(kFuncFlagIsNaked);
  if (func->getHint(kFuncHintCompact  ) != 0) func->addFuncFlags(kFuncFlagX86Leave);
  if (func->getHint(kFuncHintX86Emms  ) != 0) func->addFuncFlags(kFuncFlagX86Emms);
  if (func->getHint(kFuncHintX86SFence) != 0) func->addFuncFlags(kFuncFlagX86SFence);
  if (func->getHint(kFuncHintX86LFence) != 0) func->addFuncFlags(kFuncFlagX86LFence);

  // Global allocable registers.
  uint32_t* gaRegs = _gaRegs;

  if (!func->hasFuncFlag(kFuncFlagIsNaked))
    gaRegs[kX86RegClassGp] &= ~Utils::mask(kX86RegIndexBp);

  // Allowed index registers (Gp/Xmm/Ymm).
  const uint32_t indexMask = Utils::bits(_regCount.getGp()) & ~(Utils::mask(4, 12));

  // --------------------------------------------------------------------------
  // [VI Macros]
  // --------------------------------------------------------------------------

#define VI_BEGIN() \
  do { \
    uint32_t vaCount = 0; \
    X86RegCount regCount; \
    \
    X86RegMask inRegs; \
    X86RegMask outRegs; \
    X86RegMask clobberedRegs; \
    \
    regCount.reset(); \
    inRegs.reset(); \
    outRegs.reset(); \
    clobberedRegs.reset()

#define VI_END(_Node_) \
    if (vaCount == 0 && clobberedRegs.isEmpty()) \
      break; \
    \
    X86RegCount _vaIndex; \
    _vaIndex.indexFromRegCount(regCount); \
    \
    X86VarMap* _map = newVarMap(vaCount); \
    if (_map == nullptr) \
      goto _NoMemory; \
    \
    _map->_vaCount = vaCount; \
    _map->_count = regCount; \
    _map->_start = _vaIndex; \
    \
    _map->_inRegs = inRegs; \
    _map->_outRegs = outRegs; \
    _map->_clobberedRegs = clobberedRegs; \
    \
    VarAttr* _va = vaTmpList; \
    while (vaCount) { \
      VarData* _vd = _va->getVd(); \
      \
      uint32_t _class = _vd->getClass(); \
      uint32_t _index = _vaIndex.get(_class); \
      \
      _vaIndex.add(_class); \
      \
      if (_va->_inRegs) \
        _va->_allocableRegs = _va->_inRegs; \
      else if (_va->_outRegIndex != kInvalidReg) \
        _va->_allocableRegs = Utils::mask(_va->_outRegIndex); \
      else \
        _va->_allocableRegs &= ~inRegs.get(_class); \
      \
      _vd->_va = nullptr; \
      _map->getVa(_index)[0] = _va[0]; \
      \
      _va++; \
      vaCount--; \
    } \
    \
    _Node_->setMap(_map); \
  } while (0)

#define VI_ADD_VAR(_Vd_, _Va_, _Flags_, _NewAllocable_) \
  do { \
    ASMJIT_ASSERT(_Vd_->_va == nullptr); \
    \
    _Va_ = &vaTmpList[vaCount++]; \
    _Va_->setup(_Vd_, _Flags_, 0, _NewAllocable_); \
    _Va_->addVarCount(1); \
    _Vd_->setVa(_Va_); \
    \
    if (_registerContextVar(_Vd_) != kErrorOk) \
      goto _NoMemory; \
    regCount.add(_Vd_->getClass()); \
  } while (0)

#define VI_MERGE_VAR(_Vd_, _Va_, _Flags_, _NewAllocable_) \
  do { \
    _Va_ = _Vd_->getVa(); \
    \
    if (_Va_ == nullptr) { \
      _Va_ = &vaTmpList[vaCount++]; \
      _Va_->setup(_Vd_, 0, 0, _NewAllocable_); \
      _Vd_->setVa(_Va_); \
      \
      if (_registerContextVar(_Vd_) != kErrorOk) \
        goto _NoMemory; \
      regCount.add(_Vd_->getClass()); \
    } \
    \
    _Va_->orFlags(_Flags_); \
    _Va_->addVarCount(1); \
  } while (0)

  // --------------------------------------------------------------------------
  // [Loop]
  // --------------------------------------------------------------------------

  do {
_Do:
    while (node_->isFetched()) {
_NextGroup:
      if (jLink == nullptr)
        jLink = _jccList.getFirst();
      else
        jLink = jLink->getNext();

      if (jLink == nullptr)
        goto _Done;
      node_ = X86Context_getOppositeJccFlow(static_cast<HLJump*>(jLink->getValue()));
    }

    flowId++;

    next = node_->getNext();
    node_->setFlowId(flowId);

    ASMJIT_TSEC({
      this->_traceNode(this, node_, "[F] ");
    });

    switch (node_->getType()) {
      // ----------------------------------------------------------------------
      // [Align/Embed]
      // ----------------------------------------------------------------------

      case HLNode::kTypeAlign:
      case HLNode::kTypeData:
        break;

      // ----------------------------------------------------------------------
      // [Hint]
      // ----------------------------------------------------------------------

      case HLNode::kTypeHint: {
        HLHint* node = static_cast<HLHint*>(node_);
        VI_BEGIN();

        if (node->getHint() == kVarHintAlloc) {
          uint32_t remain[_kX86RegClassManagedCount];
          HLHint* cur = node;

          remain[kX86RegClassGp ] = _regCount.getGp() - 1 - func->hasFuncFlag(kFuncFlagIsNaked);
          remain[kX86RegClassMm ] = _regCount.getMm();
          remain[kX86RegClassK  ] = _regCount.getK();
          remain[kX86RegClassXyz] = _regCount.getXyz();

          // Merge as many alloc-hints as possible.
          for (;;) {
            VarData* vd = static_cast<VarData*>(cur->getVd());
            VarAttr* va = vd->getVa();

            uint32_t regClass = vd->getClass();
            uint32_t regIndex = cur->getValue();
            uint32_t regMask = 0;

            // We handle both kInvalidReg and kInvalidValue.
            if (regIndex < kInvalidReg)
              regMask = Utils::mask(regIndex);

            if (va == nullptr) {
              if (inRegs.has(regClass, regMask))
                break;
              if (remain[regClass] == 0)
                break;
              VI_ADD_VAR(vd, va, kVarAttrRReg, gaRegs[regClass]);

              if (regMask != 0) {
                inRegs.xor_(regClass, regMask);
                va->setInRegs(regMask);
                va->setInRegIndex(regIndex);
              }

              remain[regClass]--;
            }
            else if (regMask != 0) {
              if (inRegs.has(regClass, regMask) && va->getInRegs() != regMask)
                break;

              inRegs.xor_(regClass, va->getInRegs() | regMask);
              va->setInRegs(regMask);
              va->setInRegIndex(regIndex);
            }

            if (cur != node)
              compiler->removeNode(cur);

            cur = static_cast<HLHint*>(node->getNext());
            if (cur == nullptr || cur->getType() != HLNode::kTypeHint || cur->getHint() != kVarHintAlloc)
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
              flags = kVarAttrRMem | kVarAttrSpill;
              break;
            case kVarHintSave:
              flags = kVarAttrRMem;
              break;
            case kVarHintSaveAndUnuse:
              flags = kVarAttrRMem | kVarAttrUnuse;
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

      case HLNode::kTypeLabel: {
        if (node_ == func->getExitNode()) {
          ASMJIT_PROPAGATE_ERROR(addReturningNode(node_));
          goto _NextGroup;
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Inst]
      // ----------------------------------------------------------------------

      case HLNode::kTypeInst: {
        HLInst* node = static_cast<HLInst*>(node_);

        uint32_t instId = node->getInstId();
        uint32_t flags = node->getFlags();

        Operand* opList = node->getOpList();
        uint32_t opCount = node->getOpCount();

        if (opCount) {
          const X86InstExtendedInfo& extendedInfo = _x86InstInfo[instId].getExtendedInfo();
          const X86SpecialInst* special = nullptr;
          VI_BEGIN();

          // Collect instruction flags and merge all 'VarAttr's.
          if (extendedInfo.isFp())
            flags |= HLNode::kFlagIsFp;

          if (extendedInfo.isSpecial() && (special = X86SpecialInst_get(instId, opList, opCount)) != nullptr)
            flags |= HLNode::kFlagIsSpecial;

          uint32_t gpAllowedMask = 0xFFFFFFFF;

          for (uint32_t i = 0; i < opCount; i++) {
            Operand* op = &opList[i];
            VarData* vd;
            VarAttr* va;

            if (op->isVar()) {
              vd = compiler->getVdById(op->getId());
              VI_MERGE_VAR(vd, va, 0, gaRegs[vd->getClass()] & gpAllowedMask);

              if (static_cast<X86Var*>(op)->isGpb()) {
                va->orFlags(static_cast<X86GpVar*>(op)->isGpbLo() ? kVarAttrX86GpbLo : kVarAttrX86GpbHi);
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
                  if (static_cast<X86GpVar*>(op)->isGpbHi()) {
                    va->_allocableRegs &= 0x0F;

                    if (gpAllowedMask != 0xFF) {
                      for (uint32_t j = 0; j < i; j++)
                        vaTmpList[j]._allocableRegs &= vaTmpList[j].hasFlag(kVarAttrX86GpbHi) ? 0x0F : 0xFF;
                      gpAllowedMask = 0xFF;
                    }
                  }
                }
              }

              if (special != nullptr) {
                uint32_t inReg = special[i].inReg;
                uint32_t outReg = special[i].outReg;
                uint32_t c;

                if (static_cast<const X86Reg*>(op)->isGp())
                  c = kX86RegClassGp;
                else
                  c = kX86RegClassXyz;

                if (inReg != kInvalidReg) {
                  uint32_t mask = Utils::mask(inReg);
                  inRegs.or_(c, mask);
                  va->addInRegs(mask);
                }

                if (outReg != kInvalidReg) {
                  uint32_t mask = Utils::mask(outReg);
                  outRegs.or_(c, mask);
                  va->setOutRegIndex(outReg);
                }

                va->orFlags(special[i].flags);
              }
              else {
                uint32_t inFlags = kVarAttrRReg;
                uint32_t outFlags = kVarAttrWReg;
                uint32_t combinedFlags;

                if (i == 0) {
                  // Read/Write is usualy the combination of the first operand.
                  combinedFlags = inFlags | outFlags;

                  // Handle overwrite option.
                  if (node->getOptions() & kInstOptionOverwrite) {
                    combinedFlags = outFlags;
                  }
                  // Move instructions typically overwrite the first operand,
                  // but there are some exceptions based on the operands' size
                  // and type.
                  else if (extendedInfo.isMove()) {
                    uint32_t movSize = extendedInfo.getWriteSize();
                    uint32_t varSize = vd->getSize();

                    // Exception - If the source operand is a memory location
                    // promote move size into 16 bytes.
                    if (extendedInfo.isZeroIfMem() && opList[1].isMem())
                      movSize = 16;

                    if (static_cast<const X86Var*>(op)->isGp()) {
                      uint32_t opSize = static_cast<const X86Var*>(op)->getSize();

                      // Move size is zero in case that it should be determined
                      // from the destination register.
                      if (movSize == 0)
                        movSize = opSize;

                      // Handle the case that a 32-bit operation in 64-bit mode
                      // always zeroes the rest of the destination register and
                      // the case that move size is actually greater than or
                      // equal to the size of the variable.
                      if (movSize >= 4 || movSize >= varSize)
                        combinedFlags = outFlags;
                    }
                    else if (movSize >= varSize) {
                      // If move size is greater than or equal to the size of
                      // the variable there is nothing to do, because the move
                      // will overwrite the variable in all cases.
                      combinedFlags = outFlags;
                    }
                  }
                  // Comparison/Test instructions don't modify any operand.
                  else if (extendedInfo.isTest()) {
                    combinedFlags = inFlags;
                  }
                  // Imul.
                  else if (instId == kX86InstIdImul && opCount == 3) {
                    combinedFlags = outFlags;
                  }
                }
                else {
                  // Read-Only is usualy the combination of the second/third/fourth operands.
                  combinedFlags = inFlags;

                  // Idiv is a special instruction, never handled here.
                  ASMJIT_ASSERT(instId != kX86InstIdIdiv);

                  // Xchg/Xadd/Imul.
                  if (extendedInfo.isXchg() || (instId == kX86InstIdImul && opCount == 3 && i == 1))
                    combinedFlags = inFlags | outFlags;
                }
                va->orFlags(combinedFlags);
              }
            }
            else if (op->isMem()) {
              X86Mem* m = static_cast<X86Mem*>(op);
              node->setMemOpIndex(i);

              if (OperandUtil::isVarId(m->getBase()) && m->isBaseIndexType()) {
                vd = compiler->getVdById(m->getBase());
                if (!vd->isStack()) {
                  VI_MERGE_VAR(vd, va, 0, gaRegs[vd->getClass()] & gpAllowedMask);
                  if (m->getMemType() == kMemTypeBaseIndex) {
                    va->orFlags(kVarAttrRReg);
                  }
                  else {
                    uint32_t inFlags = kVarAttrRMem;
                    uint32_t outFlags = kVarAttrWMem;
                    uint32_t combinedFlags;

                    if (i == 0) {
                      // Default for the first operand.
                      combinedFlags = inFlags | outFlags;

                      // Move to memory - setting the right flags is important
                      // as if it's just move to the register. It's just a bit
                      // simpler as there are no special cases.
                      if (extendedInfo.isMove()) {
                        uint32_t movSize = Utils::iMax<uint32_t>(extendedInfo.getWriteSize(), m->getSize());
                        uint32_t varSize = vd->getSize();

                        if (movSize >= varSize)
                          combinedFlags = outFlags;
                      }
                      // Comparison/Test instructions don't modify any operand.
                      else if (extendedInfo.isTest()) {
                        combinedFlags = inFlags;
                      }
                    }
                    else {
                      // Default for the second operand.
                      combinedFlags = inFlags;

                      // Handle Xchg instruction (modifies both operands).
                      if (extendedInfo.isXchg())
                        combinedFlags = inFlags | outFlags;
                    }

                    va->orFlags(combinedFlags);
                  }
                }
              }

              if (OperandUtil::isVarId(m->getIndex())) {
                // Restrict allocation to all registers except ESP/RSP/R12.
                vd = compiler->getVdById(m->getIndex());
                VI_MERGE_VAR(vd, va, 0, gaRegs[kX86RegClassGp] & gpAllowedMask);
                va->andAllocableRegs(indexMask);
                va->orFlags(kVarAttrRReg);
              }
            }
          }

          node->setFlags(flags);
          if (vaCount) {
            // Handle instructions which result in zeros/ones or nop if used with the
            // same destination and source operand.
            if (vaCount == 1 && opCount >= 2 && opList[0].isVar() && opList[1].isVar() && !node->hasMemOp())
              X86Context_prepareSingleVarInst(instId, &vaTmpList[0]);
          }

          VI_END(node_);
        }

        // Handle conditional/unconditional jump.
        if (node->isJmpOrJcc()) {
          HLJump* jNode = static_cast<HLJump*>(node);
          HLLabel* jTarget = jNode->getTarget();

          // If this jump is unconditional we put next node to unreachable node
          // list so we can eliminate possible dead code. We have to do this in
          // all cases since we are unable to translate without fetch() step.
          //
          // We also advance our node pointer to the target node to simulate
          // natural flow of the function.
          if (jNode->isJmp()) {
            if (!next->isFetched())
              ASMJIT_PROPAGATE_ERROR(addUnreachableNode(next));

            // Jump not followed.
            if (jTarget == nullptr) {
              ASMJIT_PROPAGATE_ERROR(addReturningNode(jNode));
              goto _NextGroup;
            }

            node_ = jTarget;
            goto _Do;
          }
          else {
            // Jump not followed.
            if (jTarget == nullptr)
              break;

            if (jTarget->isFetched()) {
              uint32_t jTargetFlowId = jTarget->getFlowId();

              // Update HLNode::kFlagIsTaken flag to true if this is a
              // conditional backward jump. This behavior can be overridden
              // by using `kInstOptionTaken` when the instruction is created.
              if (!jNode->isTaken() && opCount == 1 && jTargetFlowId <= flowId) {
                jNode->orFlags(HLNode::kFlagIsTaken);
              }
            }
            else if (next->isFetched()) {
              node_ = jTarget;
              goto _Do;
            }
            else {
              ASMJIT_PROPAGATE_ERROR(addJccNode(jNode));
              node_ = X86Context_getJccFlow(jNode);
              goto _Do;
            }
          }
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Func]
      // ----------------------------------------------------------------------

      case HLNode::kTypeFunc: {
        ASMJIT_ASSERT(node_ == func);
        X86FuncDecl* decl = func->getDecl();

        VI_BEGIN();
        for (uint32_t i = 0, argCount = decl->getNumArgs(); i < argCount; i++) {
          const FuncInOut& arg = decl->getArg(i);

          VarData* vd = func->getArg(i);
          VarAttr* va;

          if (vd == nullptr)
            continue;

          // Overlapped function arguments.
          if (vd->getVa() != nullptr)
            return compiler->setLastError(kErrorOverlappedArgs);
          VI_ADD_VAR(vd, va, 0, 0);

          uint32_t aType = arg.getVarType();
          uint32_t vType = vd->getType();

          if (arg.hasRegIndex()) {
            if (x86VarTypeToClass(aType) == vd->getClass()) {
              va->orFlags(kVarAttrWReg);
              va->setOutRegIndex(arg.getRegIndex());
            }
            else {
              va->orFlags(kVarAttrWConv);
            }
          }
          else {
            if ((x86VarTypeToClass(aType) == vd->getClass()) ||
                (vType == kX86VarTypeXmmSs && aType == kVarTypeFp32) ||
                (vType == kX86VarTypeXmmSd && aType == kVarTypeFp64)) {
              va->orFlags(kVarAttrWMem);
            }
            else {
              // TODO: [COMPILER] Not implemented.
              ASMJIT_ASSERT(!"Implemented");
            }
          }
        }
        VI_END(node_);
        break;
      }

      // ----------------------------------------------------------------------
      // [End]
      // ----------------------------------------------------------------------

      case HLNode::kTypeSentinel: {
        ASMJIT_PROPAGATE_ERROR(addReturningNode(node_));
        goto _NextGroup;
      }

      // ----------------------------------------------------------------------
      // [Ret]
      // ----------------------------------------------------------------------

      case HLNode::kTypeRet: {
        HLRet* node = static_cast<HLRet*>(node_);
        ASMJIT_PROPAGATE_ERROR(addReturningNode(node));

        X86FuncDecl* decl = func->getDecl();
        if (decl->hasRet()) {
          const FuncInOut& ret = decl->getRet(0);
          uint32_t retClass = x86VarTypeToClass(ret.getVarType());

          VI_BEGIN();
          for (uint32_t i = 0; i < 2; i++) {
            Operand* op = &node->_ret[i];

            if (op->isVar()) {
              VarData* vd = compiler->getVdById(op->getId());
              VarAttr* va;

              VI_MERGE_VAR(vd, va, 0, 0);

              if (retClass == vd->getClass()) {
                // TODO: [COMPILER] Fix HLRet fetch.
                va->orFlags(kVarAttrRReg);
                va->setInRegs(i == 0 ? Utils::mask(kX86RegIndexAx) : Utils::mask(kX86RegIndexDx));
                inRegs.or_(retClass, va->getInRegs());
              }
              else if (retClass == kX86RegClassFp) {
                uint32_t fldFlag = ret.getVarType() == kVarTypeFp32 ? kVarAttrX86Fld4 : kVarAttrX86Fld8;
                va->orFlags(kVarAttrRMem | fldFlag);
              }
              else {
                // TODO: Fix possible other return type conversions.
                ASMJIT_NOT_REACHED();
              }
            }
          }
          VI_END(node_);
        }

        if (!next->isFetched())
          ASMJIT_PROPAGATE_ERROR(addUnreachableNode(next));
        goto _NextGroup;
      }

      // ----------------------------------------------------------------------
      // [Call]
      // ----------------------------------------------------------------------

      case HLNode::kTypeCall: {
        X86CallNode* node = static_cast<X86CallNode*>(node_);
        X86FuncDecl* decl = node->getDecl();

        Operand* target = &node->_target;
        Operand* args = node->_args;
        Operand* rets = node->_ret;

        func->addFuncFlags(kFuncFlagIsCaller);
        func->mergeCallStackSize(node->_x86Decl.getArgStackSize());
        node->_usedArgs = X86Context_getUsedArgs(this, node, decl);

        uint32_t i;
        uint32_t argCount = decl->getNumArgs();
        uint32_t sArgCount = 0;
        uint32_t gpAllocableMask = gaRegs[kX86RegClassGp] & ~node->_usedArgs.get(kX86RegClassGp);

        VarData* vd;
        VarAttr* va;

        VI_BEGIN();

        // Function-call operand.
        if (target->isVar()) {
          vd = compiler->getVdById(target->getId());
          VI_MERGE_VAR(vd, va, 0, 0);

          va->orFlags(kVarAttrRReg | kVarAttrRCall);
          if (va->getInRegs() == 0)
            va->addAllocableRegs(gpAllocableMask);
        }
        else if (target->isMem()) {
          X86Mem* m = static_cast<X86Mem*>(target);

          if (OperandUtil::isVarId(m->getBase()) && m->isBaseIndexType()) {
            vd = compiler->getVdById(m->getBase());
            if (!vd->isStack()) {
              VI_MERGE_VAR(vd, va, 0, 0);
              if (m->getMemType() == kMemTypeBaseIndex) {
                va->orFlags(kVarAttrRReg | kVarAttrRCall);
                if (va->getInRegs() == 0)
                  va->addAllocableRegs(gpAllocableMask);
              }
              else {
                va->orFlags(kVarAttrRMem | kVarAttrRCall);
              }
            }
          }

          if (OperandUtil::isVarId(m->getIndex())) {
            // Restrict allocation to all registers except ESP/RSP/R12.
            vd = compiler->getVdById(m->getIndex());
            VI_MERGE_VAR(vd, va, 0, 0);

            va->orFlags(kVarAttrRReg | kVarAttrRCall);
            if ((va->getInRegs() & ~indexMask) == 0)
              va->andAllocableRegs(gpAllocableMask & indexMask);
          }
        }

        // Function-call arguments.
        for (i = 0; i < argCount; i++) {
          Operand* op = &args[i];
          if (!op->isVar())
            continue;

          vd = compiler->getVdById(op->getId());
          const FuncInOut& arg = decl->getArg(i);

          if (arg.hasRegIndex()) {
            VI_MERGE_VAR(vd, va, 0, 0);

            uint32_t argType = arg.getVarType();
            uint32_t argClass = x86VarTypeToClass(argType);

            if (vd->getClass() == argClass) {
              va->addInRegs(Utils::mask(arg.getRegIndex()));
              va->orFlags(kVarAttrRReg | kVarAttrRFunc);
            }
            else {
              va->orFlags(kVarAttrRConv | kVarAttrRFunc);
            }
          }
          // If this is a stack-based argument we insert HLCallArg instead of
          // using VarAttr. It improves the code, because the argument can be
          // moved onto stack as soon as it is ready and the register used by
          // the variable can be reused for something else. It is also much
          // easier to handle argument conversions, because there will be at
          // most only one node per conversion.
          else {
            if (X86Context_insertHLCallArg(this, node, vd, gaRegs, arg, i, sArgList, sArgCount) != kErrorOk)
              goto _NoMemory;
          }
        }

        // Function-call return(s).
        for (i = 0; i < 2; i++) {
          Operand* op = &rets[i];
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
              va->orFlags(kVarAttrWReg | kVarAttrWFunc);
            }
            else {
              va->orFlags(kVarAttrWConv | kVarAttrWFunc);
            }
          }
        }

        // Init clobbered.
        clobberedRegs.set(kX86RegClassGp , Utils::bits(_regCount.getGp())  & (~decl->getPreserved(kX86RegClassGp )));
        clobberedRegs.set(kX86RegClassMm , Utils::bits(_regCount.getMm())  & (~decl->getPreserved(kX86RegClassMm )));
        clobberedRegs.set(kX86RegClassK  , Utils::bits(_regCount.getK())   & (~decl->getPreserved(kX86RegClassK  )));
        clobberedRegs.set(kX86RegClassXyz, Utils::bits(_regCount.getXyz()) & (~decl->getPreserved(kX86RegClassXyz)));

        VI_END(node_);
        break;
      }

      default:
        break;
    }

    node_ = next;
  } while (node_ != stop);

_Done:
  // Mark exit label and end node as fetched, otherwise they can be removed by
  // `removeUnreachableCode()`, which would lead to crash in some later step.
  node_ = func->getEnd();
  if (!node_->isFetched()) {
    func->getExitNode()->setFlowId(++flowId);
    node_->setFlowId(++flowId);
  }

  ASMJIT_TLOG("[F] ======= Fetch (Done)\n");
  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Failure]
  // --------------------------------------------------------------------------

_NoMemory:
  ASMJIT_TLOG("[F] ======= Fetch (Out of Memory)\n");
  return compiler->setLastError(kErrorNoHeapMemory);
}

// ============================================================================
// [asmjit::X86Context - Annotate]
// ============================================================================

Error X86Context::annotate() {
#if !defined(ASMJIT_DISABLE_LOGGER)
  HLFunc* func = getFunc();

  HLNode* node_ = func;
  HLNode* end = func->getEnd();

  Zone& sa = _compiler->_stringAllocator;
  StringBuilderTmp<128> sb;

  uint32_t maxLen = 0;
  while (node_ != end) {
    if (node_->getComment() == nullptr) {
      if (node_->getType() == HLNode::kTypeInst) {
        HLInst* node = static_cast<HLInst*>(node_);
        X86Context_annotateInstruction(this, sb, node->getInstId(), node->getOpList(), node->getOpCount());

        node_->setComment(static_cast<char*>(sa.dup(sb.getData(), sb.getLength() + 1)));
        maxLen = Utils::iMax<uint32_t>(maxLen, static_cast<uint32_t>(sb.getLength()));

        sb.clear();
      }
    }

    node_ = node_->getNext();
  }
  _annotationLength = maxLen + 1;
#endif // !ASMJIT_DISABLE_LOGGER

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86BaseAlloc]
// ============================================================================

struct X86BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86BaseAlloc(X86Context* context) {
    _context = context;
    _compiler = context->getCompiler();
  }
  ASMJIT_INLINE ~X86BaseAlloc() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the context.
  ASMJIT_INLINE X86Context* getContext() const { return _context; }
  //! Get the current state (always the same instance as X86Context::_x86State).
  ASMJIT_INLINE X86VarState* getState() const { return _context->getState(); }

  //! Get the node.
  ASMJIT_INLINE HLNode* getNode() const { return _node; }

  //! Get VarAttr list (all).
  ASMJIT_INLINE VarAttr* getVaList() const { return _vaList[0]; }
  //! Get VarAttr list (per class).
  ASMJIT_INLINE VarAttr* getVaListByClass(uint32_t rc) const { return _vaList[rc]; }

  //! Get VarAttr count (all).
  ASMJIT_INLINE uint32_t getVaCount() const { return _vaCount; }
  //! Get VarAttr count (per class).
  ASMJIT_INLINE uint32_t getVaCountByClass(uint32_t rc) const { return _count.get(rc); }

  //! Get whether all variables of class `c` are done.
  ASMJIT_INLINE bool isVaDone(uint32_t rc) const { return _done.get(rc) == _count.get(rc); }

  //! Get how many variables have been allocated.
  ASMJIT_INLINE uint32_t getVaDone(uint32_t rc) const { return _done.get(rc); }
  //! Add to the count of variables allocated.
  ASMJIT_INLINE void addVaDone(uint32_t rc, uint32_t n = 1) { _done.add(rc, n); }

  //! Get number of allocable registers per class.
  ASMJIT_INLINE uint32_t getGaRegs(uint32_t rc) const {
    return _context->_gaRegs[rc];
  }

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods by X86Context::translate().

  ASMJIT_INLINE void init(HLNode* node, X86VarMap* map);
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

  //! Context.
  X86Context* _context;
  //! Compiler.
  X86Compiler* _compiler;

  //! Node.
  HLNode* _node;

  //! Variable map.
  X86VarMap* _map;
  //! VarAttr list (per register class).
  VarAttr* _vaList[_kX86RegClassManagedCount];

  //! Count of all VarAttr's.
  uint32_t _vaCount;

  //! VarAttr's total counter.
  X86RegCount _count;
  //! VarAttr's done counter.
  X86RegCount _done;
};

// ============================================================================
// [asmjit::X86BaseAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86BaseAlloc::init(HLNode* node, X86VarMap* map) {
  _node = node;
  _map = map;

  // We have to set the correct cursor in case any instruction is emitted
  // during the allocation phase; it has to be emitted before the current
  // instruction.
  _compiler->_setCursor(node->getPrev());

  // Setup the lists of variables.
  {
    VarAttr* va = map->getVaList();
    _vaList[kX86RegClassGp ] = va;
    _vaList[kX86RegClassMm ] = va + map->getVaStart(kX86RegClassMm );
    _vaList[kX86RegClassK  ] = va + map->getVaStart(kX86RegClassK  );
    _vaList[kX86RegClassXyz] = va + map->getVaStart(kX86RegClassXyz);
  }

  // Setup counters.
  _vaCount = map->getVaCount();

  _count = map->_count;
  _done.reset();

  // Connect Vd->Va.
  for (uint32_t i = 0; i < _vaCount; i++) {
    VarAttr* va = &_vaList[0][i];
    VarData* vd = va->getVd();

    vd->setVa(va);
  }
}

ASMJIT_INLINE void X86BaseAlloc::cleanup() {
  // Disconnect Vd->Va.
  for (uint32_t i = 0; i < _vaCount; i++) {
    VarAttr* va = &_vaList[0][i];
    VarData* vd = va->getVd();

    vd->setVa(nullptr);
  }
}

// ============================================================================
// [asmjit::X86BaseAlloc - Unuse]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86BaseAlloc::unuseBefore() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  const uint32_t checkFlags =
    kVarAttrXReg  |
    kVarAttrRMem  |
    kVarAttrRFunc |
    kVarAttrRCall |
    kVarAttrRConv ;

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if ((va->getFlags() & checkFlags) == kVarAttrWReg) {
      _context->unuse<C>(va->getVd());
    }
  }
}

template<int C>
ASMJIT_INLINE void X86BaseAlloc::unuseAfter() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if (va->getFlags() & kVarAttrUnuse)
      _context->unuse<C>(va->getVd());
  }
}

// ============================================================================
// [asmjit::X86VarAlloc]
// ============================================================================

//! \internal
//!
//! Register allocator context (asm instructions).
struct X86VarAlloc : public X86BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86VarAlloc(X86Context* context) : X86BaseAlloc(context) {}
  ASMJIT_INLINE ~X86VarAlloc() {}

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error run(HLNode* node);

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods by X86Context::translate().

  ASMJIT_INLINE void init(HLNode* node, X86VarMap* map);
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

  //! Guess which register is the best candidate for 'vd' from
  //! 'allocableRegs'.
  //!
  //! The guess is based on looking ahead and inspecting register allocator
  //! instructions. The main reason is to prevent allocation to a register
  //! which is needed by next instruction(s). The guess look tries to go as far
  //! as possible, after the remaining registers are zero, the mask of previous
  //! registers (called 'safeRegs') is returned.
  template<int C>
  ASMJIT_INLINE uint32_t guessAlloc(VarData* vd, uint32_t allocableRegs);

  //! Guess whether to move the given 'vd' instead of spill.
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

  //! Will alloc to these registers.
  X86RegMask _willAlloc;
  //! Will spill these registers.
  X86RegMask _willSpill;
};

// ============================================================================
// [asmjit::X86VarAlloc - Run]
// ============================================================================

ASMJIT_INLINE Error X86VarAlloc::run(HLNode* node_) {
  // Initialize.
  X86VarMap* map = node_->getMap<X86VarMap>();
  if (map == nullptr)
    return kErrorOk;

  // Initialize the allocator; connect Vd->Va.
  init(node_, map);

  // Unuse overwritten variables.
  unuseBefore<kX86RegClassGp>();
  unuseBefore<kX86RegClassMm>();
  unuseBefore<kX86RegClassXyz>();

  // Plan the allocation. Planner assigns input/output registers for each
  // variable and decides whether to allocate it in register or stack.
  plan<kX86RegClassGp>();
  plan<kX86RegClassMm>();
  plan<kX86RegClassXyz>();

  // Spill all variables marked by plan().
  spill<kX86RegClassGp>();
  spill<kX86RegClassMm>();
  spill<kX86RegClassXyz>();

  // Alloc all variables marked by plan().
  alloc<kX86RegClassGp>();
  alloc<kX86RegClassMm>();
  alloc<kX86RegClassXyz>();

  // Translate node operands.
  if (node_->getType() == HLNode::kTypeInst) {
    HLInst* node = static_cast<HLInst*>(node_);
    ASMJIT_PROPAGATE_ERROR(X86Context_translateOperands(_context, node->getOpList(), node->getOpCount()));
  }
  else if (node_->getType() == HLNode::kTypeCallArg) {
    HLCallArg* node = static_cast<HLCallArg*>(node_);

    X86CallNode* call = static_cast<X86CallNode*>(node->getCall());
    X86FuncDecl* decl = call->getDecl();

    uint32_t argIndex = 0;
    uint32_t argMask = node->_args;

    VarData* sVd = node->getSVd();
    VarData* cVd = node->getCVd();

    // Convert first.
    ASMJIT_ASSERT(sVd->getRegIndex() != kInvalidReg);

    if (cVd != nullptr) {
      ASMJIT_ASSERT(cVd->getRegIndex() != kInvalidReg);
      _context->emitConvertVarToVar(
        cVd->getType(), cVd->getRegIndex(),
        sVd->getType(), sVd->getRegIndex());
      sVd = cVd;
    }

    while (argMask != 0) {
      if (argMask & 0x1) {
        FuncInOut& arg = decl->getArg(argIndex);
        ASMJIT_ASSERT(arg.hasStackOffset());

        X86Mem dst = x86::ptr(_context->_zsp, -static_cast<int>(_context->getRegSize()) + arg.getStackOffset());
        _context->emitMoveVarOnStack(arg.getVarType(), &dst, sVd->getType(), sVd->getRegIndex());
      }

      argIndex++;
      argMask >>= 1;
    }
  }

  // Mark variables as modified.
  modified<kX86RegClassGp>();
  modified<kX86RegClassMm>();
  modified<kX86RegClassXyz>();

  // Cleanup; disconnect Vd->Va.
  cleanup();

  // Update clobbered mask.
  _context->_clobberedRegs.or_(_willAlloc);
  _context->_clobberedRegs.or_(map->_clobberedRegs);

  // Unuse.
  unuseAfter<kX86RegClassGp>();
  unuseAfter<kX86RegClassMm>();
  unuseAfter<kX86RegClassXyz>();

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86VarAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86VarAlloc::init(HLNode* node, X86VarMap* map) {
  X86BaseAlloc::init(node, map);

  // These will block planner from assigning them during planning. Planner will
  // add more registers when assigning registers to variables that don't need
  // any specific register.
  _willAlloc = map->_inRegs;
  _willAlloc.or_(map->_outRegs);
  _willSpill.reset();
}

ASMJIT_INLINE void X86VarAlloc::cleanup() {
  X86BaseAlloc::cleanup();
}

// ============================================================================
// [asmjit::X86VarAlloc - Plan / Spill / Alloc]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86VarAlloc::plan() {
  if (isVaDone(C))
    return;

  uint32_t i;
  uint32_t willAlloc = _willAlloc.get(C);
  uint32_t willFree = 0;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);
  X86VarState* state = getState();

  // Calculate 'willAlloc' and 'willFree' masks based on mandatory masks.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();
    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = (regIndex != kInvalidReg) ? Utils::mask(regIndex) : 0;

    if ((vaFlags & kVarAttrXReg) != 0) {
      // Planning register allocation. First check whether the variable is
      // already allocated in register and if it can stay allocated there.
      //
      // The following conditions may happen:
      //
      // a) Allocated register is one of the mandatoryRegs.
      // b) Allocated register is one of the allocableRegs.
      uint32_t mandatoryRegs = va->getInRegs();
      uint32_t allocableRegs = va->getAllocableRegs();

      ASMJIT_TLOG("[RA-PLAN] %s (%s)\n",
        vd->getName(),
        (vaFlags & kVarAttrXReg) == kVarAttrWReg ? "R-Reg" : "X-Reg");

      ASMJIT_TLOG("[RA-PLAN] RegMask=%08X Mandatory=%08X Allocable=%08X\n",
        regMask, mandatoryRegs, allocableRegs);

      if (regMask != 0) {
        // Special path for planning output-only registers.
        if ((vaFlags & kVarAttrXReg) == kVarAttrWReg) {
          uint32_t outRegIndex = va->getOutRegIndex();
          mandatoryRegs = (outRegIndex != kInvalidReg) ? Utils::mask(outRegIndex) : 0;

          if ((mandatoryRegs | allocableRegs) & regMask) {
            va->setOutRegIndex(regIndex);
            va->orFlags(kVarAttrAllocWDone);

            if (mandatoryRegs & regMask) {
              // Case 'a' - 'willAlloc' contains initially all inRegs from all VarAttr's.
              ASMJIT_ASSERT((willAlloc & regMask) != 0);
            }
            else {
              // Case 'b'.
              va->setOutRegIndex(regIndex);
              willAlloc |= regMask;
            }

            ASMJIT_TLOG("[RA-PLAN] WillAlloc\n");
            addVaDone(C);

            continue;
          }
        }
        else {
          if ((mandatoryRegs | allocableRegs) & regMask) {
            va->setInRegIndex(regIndex);
            va->orFlags(kVarAttrAllocRDone);

            if (mandatoryRegs & regMask) {
              // Case 'a' - 'willAlloc' contains initially all inRegs from all VarAttr's.
              ASMJIT_ASSERT((willAlloc & regMask) != 0);
            }
            else {
              // Case 'b'.
              va->addInRegs(regMask);
              willAlloc |= regMask;
            }

            ASMJIT_TLOG("[RA-PLAN] WillAlloc\n");
            addVaDone(C);

            continue;
          }
        }

        // Trace it here so we don't pollute log by `WillFree` of zero regMask.
        ASMJIT_TLOG("[RA-PLAN] WillFree\n");
      }

      // Variable is not allocated or allocated in register that doesn't
      // match inRegs or allocableRegs. The next step is to pick the best
      // register for this variable. If `inRegs` contains any register the
      // decision is simple - we have to follow, in other case will use
      // the advantage of `guessAlloc()` to find a register (or registers)
      // by looking ahead. But the best way to find a good register is not
      // here since now we have no information about the registers that
      // will be freed. So instead of finding register here, we just mark
      // the current register (if variable is allocated) as `willFree` so
      // the planner can use this information in the second step to plan the
      // allocation as a whole.
      willFree |= regMask;
      continue;
    }
    else {
      // Memory access - if variable is allocated it has to be freed.
      ASMJIT_TLOG("[RA-PLAN] %s (Memory)\n", vd->getName());

      if (regMask != 0) {
        ASMJIT_TLOG("[RA-PLAN] WillFree\n");
        willFree |= regMask;
        continue;
      }
      else {
        ASMJIT_TLOG("[RA-PLAN] Done\n");
        va->orFlags(kVarAttrAllocRDone);
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

    if ((vaFlags & kVarAttrXReg) != 0) {
      if ((vaFlags & kVarAttrXReg) == kVarAttrWReg) {
        if (vaFlags & kVarAttrAllocWDone)
          continue;

        // Skip all registers that have assigned outRegIndex. Spill if occupied.
        if (va->hasOutRegIndex()) {
          uint32_t outRegs = Utils::mask(va->getOutRegIndex());
          willSpill |= occupied & outRegs;
          continue;
        }
      }
      else {
        if (vaFlags & kVarAttrAllocRDone)
          continue;

        // We skip all registers that have assigned inRegIndex, indicates that
        // the register to allocate in is known.
        if (va->hasInRegIndex()) {
          uint32_t inRegs = va->getInRegs();
          willSpill |= occupied & inRegs;
          continue;
        }
      }

      uint32_t m = va->getInRegs();
      if (va->hasOutRegIndex())
        m |= Utils::mask(va->getOutRegIndex());

      m = va->getAllocableRegs() & ~(willAlloc ^ m);
      m = guessAlloc<C>(vd, m);
      ASMJIT_ASSERT(m != 0);

      uint32_t candidateRegs = m & ~occupied;
      uint32_t homeMask = vd->getHomeMask();

      uint32_t regIndex;
      uint32_t regMask;

      if (candidateRegs == 0) {
        candidateRegs = m & occupied & ~state->_modified.get(C);
        if (candidateRegs == 0)
          candidateRegs = m;
      }

      // printf("CANDIDATE: %s %08X\n", vd->getName(), homeMask);
      if (candidateRegs & homeMask)
        candidateRegs &= homeMask;

      regIndex = Utils::findFirstBit(candidateRegs);
      regMask = Utils::mask(regIndex);

      if ((vaFlags & kVarAttrXReg) == kVarAttrWReg) {
        va->setOutRegIndex(regIndex);
      }
      else {
        va->setInRegIndex(regIndex);
        va->setInRegs(regMask);
      }

      willAlloc |= regMask;
      willSpill |= regMask & occupied;
      willFree  &=~regMask;
      occupied  |= regMask;

      continue;
    }
    else if ((vaFlags & kVarAttrXMem) != 0) {
      uint32_t regIndex = vd->getRegIndex();
      if (regIndex != kInvalidReg && (vaFlags & kVarAttrXMem) != kVarAttrWMem) {
        willSpill |= Utils::mask(regIndex);
      }
    }
  }

  // Set calculated masks back to the allocator; needed by spill() and alloc().
  _willSpill.set(C, willSpill);
  _willAlloc.set(C, willAlloc);
}

template<int C>
ASMJIT_INLINE void X86VarAlloc::spill() {
  uint32_t m = _willSpill.get(C);
  uint32_t i = static_cast<uint32_t>(0) - 1;

  if (m == 0)
    return;

  X86VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Available registers for decision if move has any benefit over spill.
  uint32_t availableRegs = getGaRegs(C) & ~(state->_occupied.get(C) | m | _willAlloc.get(C));

  do {
    // We always advance one more to destroy the bit that we have found.
    uint32_t bitIndex = Utils::findFirstBit(m) + 1;

    i += bitIndex;
    m >>= bitIndex;

    VarData* vd = sVars[i];
    ASMJIT_ASSERT(vd != nullptr);

    VarAttr* va = vd->getVa();
    ASMJIT_ASSERT(va == nullptr || !va->hasFlag(kVarAttrXReg));

    if (vd->isModified() && availableRegs) {
      // Don't check for alternatives if the variable has to be spilled.
      if (va == nullptr || !va->hasFlag(kVarAttrSpill)) {
        uint32_t altRegs = guessSpill<C>(vd, availableRegs);

        if (altRegs != 0) {
          uint32_t regIndex = Utils::findFirstBit(altRegs);
          uint32_t regMask = Utils::mask(regIndex);

          _context->move<C>(vd, regIndex);
          availableRegs ^= regMask;
          continue;
        }
      }
    }

    _context->spill<C>(vd);
  } while (m != 0);
}

template<int C>
ASMJIT_INLINE void X86VarAlloc::alloc() {
  if (isVaDone(C))
    return;

  uint32_t i;
  bool didWork;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  // Alloc 'in' regs.
  do {
    didWork = false;
    for (i = 0; i < count; i++) {
      VarAttr* aVa = &list[i];
      VarData* aVd = aVa->getVd();

      if ((aVa->getFlags() & (kVarAttrRReg | kVarAttrAllocRDone)) != kVarAttrRReg)
        continue;

      uint32_t aIndex = aVd->getRegIndex();
      uint32_t bIndex = aVa->getInRegIndex();

      // Shouldn't be the same.
      ASMJIT_ASSERT(aIndex != bIndex);

      VarData* bVd = getState()->getListByClass(C)[bIndex];
      if (bVd != nullptr) {
        // Gp registers only - Swap two registers if we can solve two
        // allocation tasks by a single 'xchg' instruction, swapping
        // two registers required by the instruction/node or one register
        // required with another non-required.
        if (C == kX86RegClassGp && aIndex != kInvalidReg) {
          VarAttr* bVa = bVd->getVa();
          _context->swapGp(aVd, bVd);

          aVa->orFlags(kVarAttrAllocRDone);
          addVaDone(C);

          // Doublehit, two registers allocated by a single swap.
          if (bVa != nullptr && bVa->getInRegIndex() == aIndex) {
            bVa->orFlags(kVarAttrAllocRDone);
            addVaDone(C);
          }

          didWork = true;
          continue;
        }
      }
      else if (aIndex != kInvalidReg) {
        _context->move<C>(aVd, bIndex);

        aVa->orFlags(kVarAttrAllocRDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
      else {
        _context->alloc<C>(aVd, bIndex);

        aVa->orFlags(kVarAttrAllocRDone);
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

    if ((va->getFlags() & (kVarAttrXReg | kVarAttrAllocWDone)) != kVarAttrWReg)
      continue;

    uint32_t regIndex = va->getOutRegIndex();
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    if (vd->getRegIndex() != regIndex) {
      ASMJIT_ASSERT(getState()->getListByClass(C)[regIndex] == nullptr);
      _context->attach<C>(vd, regIndex, false);
    }

    va->orFlags(kVarAttrAllocWDone);
    addVaDone(C);
  }
}

// ============================================================================
// [asmjit::X86VarAlloc - GuessAlloc / GuessSpill]
// ============================================================================

#if 0
// TODO: This works, but should be improved a bit. The idea is to follow code
// flow and to restrict the possible registers where to allocate as much as
// possible so we won't allocate to a register which is home of some variable
// that's gonna be used together with `vd`. The previous implementation didn't
// care about it and produced suboptimal results even in code which didn't
// require any allocs & spills.
enum { kMaxGuessFlow = 10 };

struct GuessFlowData {
  ASMJIT_INLINE void init(HLNode* node, uint32_t counter, uint32_t safeRegs) {
    _node = node;
    _counter = counter;
    _safeRegs = safeRegs;
  }

  //! Node to start.
  HLNode* _node;
  //! Number of instructions processed from the beginning.
  uint32_t _counter;
  //! Safe registers, which can be used for the allocation.
  uint32_t _safeRegs;
};

template<int C>
ASMJIT_INLINE uint32_t X86VarAlloc::guessAlloc(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_TLOG("[RA-GUESS] === %s (Input=%08X) ===\n", vd->getName(), allocableRegs);
  ASMJIT_ASSERT(allocableRegs != 0);

  return allocableRegs;

  // Stop now if there is only one bit (register) set in `allocableRegs` mask.
  uint32_t safeRegs = allocableRegs;
  if (Utils::isPowerOf2(safeRegs))
    return safeRegs;

  uint32_t counter = 0;
  uint32_t maxInst = _compiler->getMaxLookAhead();

  uint32_t localId = vd->getLocalId();
  uint32_t localToken = _compiler->_generateUniqueToken();

  uint32_t gfIndex = 0;
  GuessFlowData gfArray[kMaxGuessFlow];

  HLNode* node = _node;

  // Mark this node and also exit node, it will terminate the loop if encountered.
  node->setTokenId(localToken);
  _context->getFunc()->getExitNode()->setTokenId(localToken);

  // TODO: I don't like this jump, maybe some refactor would help to eliminate it.
  goto _Advance;

  // Look ahead and calculate mask of special registers on both - input/output.
  for (;;) {
    do {
      ASMJIT_TSEC({
        _context->_traceNode(_context, node, "  ");
      });

      // Terminate if we have seen this node already.
      if (node->hasTokenId(localToken))
        break;

      node->setTokenId(localToken);
      counter++;

      // Terminate if the variable is dead here.
      if (node->hasLiveness() && !node->getLiveness()->getBit(localId)) {
        ASMJIT_TLOG("[RA-GUESS] %s (Terminating, Not alive here)\n", vd->getName());
        break;
      }

      if (node->hasState()) {
        // If this node contains a state, we have to consider only the state
        // and then we can terminate safely - this happens if we jumped to a
        // label that is backward (i.e. start of the loop). If we survived
        // the liveness check it means that the variable is actually used.
        X86VarState* state = node->getState<X86VarState>();
        uint32_t homeRegs = 0;
        uint32_t tempRegs = 0;

        VarData** vdArray = state->getListByClass(C);
        uint32_t vdCount = _compiler->getRegCount().get(C);

        for (uint32_t vdIndex = 0; vdIndex < vdCount; vdIndex++) {
          if (vdArray[vdIndex] != nullptr)
            tempRegs |= Utils::mask(vdIndex);

          if (vdArray[vdIndex] == vd)
            homeRegs = Utils::mask(vdIndex);
        }

        tempRegs = safeRegs & ~tempRegs;
        if (!tempRegs)
          goto _Done;
        safeRegs = tempRegs;

        tempRegs = safeRegs & homeRegs;
        if (!tempRegs)
          goto _Done;
        safeRegs = tempRegs;

        goto _Done;
      }
      else {
        // Process the current node if it has any variables associated in.
        X86VarMap* map = node->getMap<X86VarMap>();
        if (map != nullptr) {
          VarAttr* vaList = map->getVaListByClass(C);
          uint32_t vaCount = map->getVaCountByClass(C);

          uint32_t homeRegs = 0;
          uint32_t tempRegs = safeRegs;
          bool found = false;

          for (uint32_t vaIndex = 0; vaIndex < vaCount; vaIndex++) {
            VarAttr* va = &vaList[vaIndex];

            if (va->getVd() == vd) {
              found = true;

              // Terminate if the variable is overwritten here.
              if (!(va->getFlags() & kVarAttrRAll))
                goto _Done;

              uint32_t mask = va->getAllocableRegs();
              if (mask != 0) {
                tempRegs &= mask;
                if (!tempRegs)
                  goto _Done;
                safeRegs = tempRegs;
              }

              mask = va->getInRegs();
              if (mask != 0) {
                tempRegs &= mask;
                if (!tempRegs)
                  goto _Done;

                safeRegs = tempRegs;
                goto _Done;
              }
            }
            else {
              // It happens often that one variable is used across many blocks of
              // assembly code. It can sometimes cause one variable to be allocated
              // in a different register, which can cause state switch to generate
              // moves in case of jumps and state intersections. We try to prevent
              // this case by also considering variables' home registers.
              homeRegs |= va->getVd()->getHomeMask();
            }
          }

          tempRegs &= ~(map->_outRegs.get(C) | map->_clobberedRegs.get(C));
          if (!found)
            tempRegs &= ~map->_inRegs.get(C);

          if (!tempRegs)
            goto _Done;
          safeRegs = tempRegs;

          if (homeRegs) {
            tempRegs = safeRegs & ~homeRegs;
            if (!tempRegs)
              goto _Done;
            safeRegs = tempRegs;
          }
        }
      }

_Advance:
      // Terminate if this is a return node.
      if (node->hasFlag(HLNode::kFlagIsRet))
        goto _Done;

      // Advance on non-conditional jump.
      if (node->hasFlag(HLNode::kFlagIsJmp)) {
        // Stop on a jump that is not followed.
        node = static_cast<HLJump*>(node)->getTarget();
        if (node == nullptr)
          break;
        continue;
      }

      // Split flow on a conditional jump.
      if (node->hasFlag(HLNode::kFlagIsJcc)) {
        // Put the next node on the stack and follow the target if possible.
        HLNode* next = node->getNext();
        if (next != nullptr && gfIndex < kMaxGuessFlow)
          gfArray[gfIndex++].init(next, counter, safeRegs);

        node = static_cast<HLJump*>(node)->getTarget();
        if (node == nullptr)
          break;
        continue;
      }

      node = node->getNext();
      ASMJIT_ASSERT(node != nullptr);
    } while (counter < maxInst);

_Done:
    for (;;) {
      if (gfIndex == 0)
        goto _Ret;

      GuessFlowData* data = &gfArray[--gfIndex];
      node = data->_node;
      counter = data->_counter;

      uint32_t tempRegs = safeRegs & data->_safeRegs;
      if (!tempRegs)
        continue;

      safeRegs = tempRegs;
      break;
    }
  }

_Ret:
  ASMJIT_TLOG("[RA-GUESS] === %s (Output=%08X) ===\n", vd->getName(), safeRegs);
  return safeRegs;
}
#endif

template<int C>
ASMJIT_INLINE uint32_t X86VarAlloc::guessAlloc(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  // Stop now if there is only one bit (register) set in `allocableRegs` mask.
  if (Utils::isPowerOf2(allocableRegs))
    return allocableRegs;

  uint32_t localId = vd->getLocalId();
  uint32_t safeRegs = allocableRegs;

  uint32_t i;
  uint32_t maxLookAhead = _compiler->getMaxLookAhead();

  // Look ahead and calculate mask of special registers on both - input/output.
  HLNode* node = _node;
  for (i = 0; i < maxLookAhead; i++) {
    BitArray* liveness = node->getLiveness();

    // If the variable becomes dead it doesn't make sense to continue.
    if (liveness != nullptr && !liveness->getBit(localId))
      break;

    // Stop on `HLSentinel` and `HLRet`.
    if (node->hasFlag(HLNode::kFlagIsRet))
      break;

    // Stop on conditional jump, we don't follow them.
    if (node->hasFlag(HLNode::kFlagIsJcc))
      break;

    // Advance on non-conditional jump.
    if (node->hasFlag(HLNode::kFlagIsJmp)) {
      node = static_cast<HLJump*>(node)->getTarget();
      // Stop on jump that is not followed.
      if (node == nullptr)
        break;
    }

    node = node->getNext();
    ASMJIT_ASSERT(node != nullptr);

    X86VarMap* map = node->getMap<X86VarMap>();
    if (map != nullptr) {
      VarAttr* va = map->findVaByClass(C, vd);
      uint32_t mask;

      if (va != nullptr) {
        // If the variable is overwritten it doesn't mase sense to continue.
        if (!(va->getFlags() & kVarAttrRAll))
          break;

        mask = va->getAllocableRegs();
        if (mask != 0) {
          allocableRegs &= mask;
          if (allocableRegs == 0)
            break;
          safeRegs = allocableRegs;
        }

        mask = va->getInRegs();
        if (mask != 0) {
          allocableRegs &= mask;
          if (allocableRegs == 0)
            break;
          safeRegs = allocableRegs;
          break;
        }

        allocableRegs &= ~(map->_outRegs.get(C) | map->_clobberedRegs.get(C));
        if (allocableRegs == 0)
          break;
      }
      else {
        allocableRegs &= ~(map->_inRegs.get(C) | map->_outRegs.get(C) | map->_clobberedRegs.get(C));
        if (allocableRegs == 0)
          break;
      }

      safeRegs = allocableRegs;
    }
  }

  return safeRegs;
}

template<int C>
ASMJIT_INLINE uint32_t X86VarAlloc::guessSpill(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  return 0;
}

// ============================================================================
// [asmjit::X86VarAlloc - Modified]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86VarAlloc::modified() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];

    if (va->hasFlag(kVarAttrWReg)) {
      VarData* vd = va->getVd();

      uint32_t regIndex = vd->getRegIndex();
      uint32_t regMask = Utils::mask(regIndex);

      vd->setModified(true);
      _context->_x86State._modified.or_(C, regMask);
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc]
// ============================================================================

//! \internal
//!
//! Register allocator context (function call).
struct X86CallAlloc : public X86BaseAlloc {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86CallAlloc(X86Context* context) : X86BaseAlloc(context) {}
  ASMJIT_INLINE ~X86CallAlloc() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the node.
  ASMJIT_INLINE X86CallNode* getNode() const { return static_cast<X86CallNode*>(_node); }

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error run(X86CallNode* node);

  // --------------------------------------------------------------------------
  // [Init / Cleanup]
  // --------------------------------------------------------------------------

protected:
  // Just to prevent calling these methods from X86Context::translate().
  ASMJIT_INLINE void init(X86CallNode* node, X86VarMap* map);
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
  // [AllocImmsOnStack]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void allocImmsOnStack();

  // --------------------------------------------------------------------------
  // [Duplicate]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void duplicate();

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
  // [Members]
  // --------------------------------------------------------------------------

  //! Will alloc to these registers.
  X86RegMask _willAlloc;
  //! Will spill these registers.
  X86RegMask _willSpill;
};

// ============================================================================
// [asmjit::X86CallAlloc - Run]
// ============================================================================

ASMJIT_INLINE Error X86CallAlloc::run(X86CallNode* node) {
  // Initialize.
  X86VarMap* map = node->getMap<X86VarMap>();
  if (map == nullptr)
    return kErrorOk;

  // Initialize the allocator; prepare basics and connect Vd->Va.
  init(node, map);

  // Plan register allocation. Planner is only able to assign one register per
  // variable. If any variable is used multiple times it will be handled later.
  plan<kX86RegClassGp >();
  plan<kX86RegClassMm >();
  plan<kX86RegClassXyz>();

  // Spill.
  spill<kX86RegClassGp >();
  spill<kX86RegClassMm >();
  spill<kX86RegClassXyz>();

  // Alloc.
  alloc<kX86RegClassGp >();
  alloc<kX86RegClassMm >();
  alloc<kX86RegClassXyz>();

  // Unuse clobbered registers that are not used to pass function arguments and
  // save variables used to pass function arguments that will be reused later on.
  save<kX86RegClassGp >();
  save<kX86RegClassMm >();
  save<kX86RegClassXyz>();

  // Allocate immediates in registers and on the stack.
  allocImmsOnStack();

  // Duplicate.
  duplicate<kX86RegClassGp >();
  duplicate<kX86RegClassMm >();
  duplicate<kX86RegClassXyz>();

  // Translate call operand.
  ASMJIT_PROPAGATE_ERROR(X86Context_translateOperands(_context, &node->_target, 1));

  // To emit instructions after call.
  _compiler->_setCursor(node);

  // If the callee pops stack it has to be manually adjusted back.
  X86FuncDecl* decl = node->getDecl();
  if (decl->getCalleePopsStack() && decl->getArgStackSize() != 0) {
    _compiler->emit(kX86InstIdSub, _context->_zsp, static_cast<int>(decl->getArgStackSize()));
  }

  // Clobber.
  clobber<kX86RegClassGp >();
  clobber<kX86RegClassMm >();
  clobber<kX86RegClassXyz>();

  // Return.
  ret();

  // Unuse.
  unuseAfter<kX86RegClassGp >();
  unuseAfter<kX86RegClassMm >();
  unuseAfter<kX86RegClassXyz>();

  // Cleanup; disconnect Vd->Va.
  cleanup();

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86CallAlloc - Init / Cleanup]
// ============================================================================

ASMJIT_INLINE void X86CallAlloc::init(X86CallNode* node, X86VarMap* map) {
  X86BaseAlloc::init(node, map);

  // Create mask of all registers that will be used to pass function arguments.
  _willAlloc = node->_usedArgs;
  _willSpill.reset();
}

ASMJIT_INLINE void X86CallAlloc::cleanup() {
  X86BaseAlloc::cleanup();
}

// ============================================================================
// [asmjit::X86CallAlloc - Plan / Spill / Alloc]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86CallAlloc::plan() {
  uint32_t i;
  uint32_t clobbered = _map->_clobberedRegs.get(C);

  uint32_t willAlloc = _willAlloc.get(C);
  uint32_t willFree = clobbered & ~willAlloc;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  X86VarState* state = getState();

  // Calculate 'willAlloc' and 'willFree' masks based on mandatory masks.
  for (i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    VarData* vd = va->getVd();

    uint32_t vaFlags = va->getFlags();
    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = (regIndex != kInvalidReg) ? Utils::mask(regIndex) : 0;

    if ((vaFlags & kVarAttrRReg) != 0) {
      // Planning register allocation. First check whether the variable is
      // already allocated in register and if it can stay there. Function
      // arguments are passed either in a specific register or in stack so
      // we care mostly of mandatory registers.
      uint32_t inRegs = va->getInRegs();

      if (inRegs == 0) {
        inRegs = va->getAllocableRegs();
      }

      // Optimize situation where the variable has to be allocated in a
      // mandatory register, but it's already allocated in register that
      // is not clobbered (i.e. it will survive function call).
      if ((regMask & inRegs) != 0 || ((regMask & ~clobbered) != 0 && (vaFlags & kVarAttrUnuse) == 0)) {
        va->setInRegIndex(regIndex);
        va->orFlags(kVarAttrAllocRDone);
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
        va->orFlags(kVarAttrAllocRDone);
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
    if ((vaFlags & kVarAttrAllocRDone) != 0 || (vaFlags & kVarAttrRReg) == 0)
      continue;

    // All registers except Gp used by call itself must have inRegIndex.
    uint32_t m = va->getInRegs();
    if (C != kX86RegClassGp || m) {
      ASMJIT_ASSERT(m != 0);
      va->setInRegIndex(Utils::findFirstBit(m));
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

    if (!(vaFlags & (kVarAttrWReg | kVarAttrUnuse)) && (candidateRegs & ~clobbered))
      candidateRegs &= ~clobbered;

    uint32_t regIndex = Utils::findFirstBit(candidateRegs);
    uint32_t regMask = Utils::mask(regIndex);

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
ASMJIT_INLINE void X86CallAlloc::spill() {
  uint32_t m = _willSpill.get(C);
  uint32_t i = static_cast<uint32_t>(0) - 1;

  if (m == 0)
    return;

  X86VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  // Available registers for decision if move has any benefit over spill.
  uint32_t availableRegs = getGaRegs(C) & ~(state->_occupied.get(C) | m | _willAlloc.get(C));

  do {
    // We always advance one more to destroy the bit that we have found.
    uint32_t bitIndex = Utils::findFirstBit(m) + 1;

    i += bitIndex;
    m >>= bitIndex;

    VarData* vd = sVars[i];
    ASMJIT_ASSERT(vd != nullptr);
    ASMJIT_ASSERT(vd->getVa() == nullptr);

    if (vd->isModified() && availableRegs) {
      uint32_t available = guessSpill<C>(vd, availableRegs);
      if (available != 0) {
        uint32_t regIndex = Utils::findFirstBit(available);
        uint32_t regMask = Utils::mask(regIndex);

        _context->move<C>(vd, regIndex);
        availableRegs ^= regMask;
        continue;
      }
    }

    _context->spill<C>(vd);
  } while (m != 0);
}

template<int C>
ASMJIT_INLINE void X86CallAlloc::alloc() {
  if (isVaDone(C))
    return;

  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  X86VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  bool didWork;

  do {
    didWork = false;
    for (i = 0; i < count; i++) {
      VarAttr* aVa = &list[i];
      VarData* aVd = aVa->getVd();

      if ((aVa->getFlags() & (kVarAttrRReg | kVarAttrAllocRDone)) != kVarAttrRReg)
        continue;

      uint32_t aIndex = aVd->getRegIndex();
      uint32_t bIndex = aVa->getInRegIndex();

      // Shouldn't be the same.
      ASMJIT_ASSERT(aIndex != bIndex);

      VarData* bVd = getState()->getListByClass(C)[bIndex];
      if (bVd != nullptr) {
        VarAttr* bVa = bVd->getVa();

        // Gp registers only - Swap two registers if we can solve two
        // allocation tasks by a single 'xchg' instruction, swapping
        // two registers required by the instruction/node or one register
        // required with another non-required.
        if (C == kX86RegClassGp) {
          _context->swapGp(aVd, bVd);

          aVa->orFlags(kVarAttrAllocRDone);
          addVaDone(C);

          // Doublehit, two registers allocated by a single swap.
          if (bVa != nullptr && bVa->getInRegIndex() == aIndex) {
            bVa->orFlags(kVarAttrAllocRDone);
            addVaDone(C);
          }

          didWork = true;
          continue;
        }
      }
      else if (aIndex != kInvalidReg) {
        _context->move<C>(aVd, bIndex);
        _context->_clobberedRegs.or_(C, Utils::mask(bIndex));

        aVa->orFlags(kVarAttrAllocRDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
      else {
        _context->alloc<C>(aVd, bIndex);
        _context->_clobberedRegs.or_(C, Utils::mask(bIndex));

        aVa->orFlags(kVarAttrAllocRDone);
        addVaDone(C);

        didWork = true;
        continue;
      }
    }
  } while (didWork);
}

// ============================================================================
// [asmjit::X86CallAlloc - AllocImmsOnStack]
// ============================================================================

ASMJIT_INLINE void X86CallAlloc::allocImmsOnStack() {
  X86CallNode* node = getNode();
  X86FuncDecl* decl = node->getDecl();

  uint32_t argCount = decl->getNumArgs();
  Operand* args = node->_args;

  for (uint32_t i = 0; i < argCount; i++) {
    Operand& op = args[i];

    if (!op.isImm())
      continue;

    const Imm& imm = static_cast<const Imm&>(op);
    const FuncInOut& arg = decl->getArg(i);
    uint32_t varType = arg.getVarType();

    if (arg.hasStackOffset()) {
      X86Mem dst = x86::ptr(_context->_zsp, -static_cast<int>(_context->getRegSize()) + arg.getStackOffset());
      _context->emitMoveImmOnStack(varType, &dst, &imm);
    }
    else {
      _context->emitMoveImmToReg(varType, arg.getRegIndex(), &imm);
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc - Duplicate]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86CallAlloc::duplicate() {
  VarAttr* list = getVaListByClass(C);
  uint32_t count = getVaCountByClass(C);

  for (uint32_t i = 0; i < count; i++) {
    VarAttr* va = &list[i];
    if (!va->hasFlag(kVarAttrRReg))
      continue;

    uint32_t inRegs = va->getInRegs();
    if (!inRegs)
      continue;

    VarData* vd = va->getVd();
    uint32_t regIndex = vd->getRegIndex();

    ASMJIT_ASSERT(regIndex != kInvalidReg);

    inRegs &= ~Utils::mask(regIndex);
    if (!inRegs)
      continue;

    for (uint32_t dupIndex = 0; inRegs != 0; dupIndex++, inRegs >>= 1) {
      if (inRegs & 0x1) {
        _context->emitMove(vd, dupIndex, regIndex, "Duplicate");
        _context->_clobberedRegs.or_(C, Utils::mask(dupIndex));
      }
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc - GuessAlloc / GuessSpill]
// ============================================================================

template<int C>
ASMJIT_INLINE uint32_t X86CallAlloc::guessAlloc(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  // Stop now if there is only one bit (register) set in 'allocableRegs' mask.
  if (Utils::isPowerOf2(allocableRegs))
    return allocableRegs;

  uint32_t i;
  uint32_t safeRegs = allocableRegs;
  uint32_t maxLookAhead = _compiler->getMaxLookAhead();

  // Look ahead and calculate mask of special registers on both - input/output.
  HLNode* node = _node;
  for (i = 0; i < maxLookAhead; i++) {
    // Stop on 'HLRet' and 'HLSentinel.
    if (node->hasFlag(HLNode::kFlagIsRet))
      break;

    // Stop on conditional jump, we don't follow them.
    if (node->hasFlag(HLNode::kFlagIsJcc))
      break;

    // Advance on non-conditional jump.
    if (node->hasFlag(HLNode::kFlagIsJmp)) {
      node = static_cast<HLJump*>(node)->getTarget();
      // Stop on jump that is not followed.
      if (node == nullptr)
        break;
    }

    node = node->getNext();
    ASMJIT_ASSERT(node != nullptr);

    X86VarMap* map = node->getMap<X86VarMap>();
    if (map != nullptr) {
      VarAttr* va = map->findVaByClass(C, vd);
      if (va != nullptr) {
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
      allocableRegs &= ~(map->_inRegs.get(C) | map->_outRegs.get(C) | map->_clobberedRegs.get(C));

      if (allocableRegs == 0)
        break;
    }
  }

_UseSafeRegs:
  return safeRegs;
}

template<int C>
ASMJIT_INLINE uint32_t X86CallAlloc::guessSpill(VarData* vd, uint32_t allocableRegs) {
  ASMJIT_ASSERT(allocableRegs != 0);

  return 0;
}

// ============================================================================
// [asmjit::X86CallAlloc - Save]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86CallAlloc::save() {
  X86VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  uint32_t affected = _map->_clobberedRegs.get(C) & state->_occupied.get(C) & state->_modified.get(C);

  for (i = 0; affected != 0; i++, affected >>= 1) {
    if (affected & 0x1) {
      VarData* vd = sVars[i];
      ASMJIT_ASSERT(vd != nullptr);
      ASMJIT_ASSERT(vd->isModified());

      VarAttr* va = vd->getVa();
      if (va == nullptr || (va->getFlags() & (kVarAttrWReg | kVarAttrUnuse)) == 0) {
        _context->save<C>(vd);
      }
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc - Clobber]
// ============================================================================

template<int C>
ASMJIT_INLINE void X86CallAlloc::clobber() {
  X86VarState* state = getState();
  VarData** sVars = state->getListByClass(C);

  uint32_t i;
  uint32_t affected = _map->_clobberedRegs.get(C) & state->_occupied.get(C);

  for (i = 0; affected != 0; i++, affected >>= 1) {
    if (affected & 0x1) {
      VarData* vd = sVars[i];
      ASMJIT_ASSERT(vd != nullptr);

      VarAttr* va = vd->getVa();
      uint32_t vdState = kVarStateNone;

      if (!vd->isModified() || (va != nullptr && (va->getFlags() & (kVarAttrWAll | kVarAttrUnuse)) != 0)) {
        vdState = kVarStateMem;
      }

      _context->unuse<C>(vd, vdState);
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc - Ret]
// ============================================================================

ASMJIT_INLINE void X86CallAlloc::ret() {
  X86CallNode* node = getNode();
  X86FuncDecl* decl = node->getDecl();

  uint32_t i;
  Operand* rets = node->_ret;

  for (i = 0; i < 2; i++) {
    const FuncInOut& ret = decl->getRet(i);
    Operand* op = &rets[i];

    if (!ret.hasRegIndex() || !op->isVar())
      continue;

    VarData* vd = _compiler->getVdById(op->getId());
    uint32_t vf = _x86VarInfo[vd->getType()].getFlags();
    uint32_t regIndex = ret.getRegIndex();

    switch (vd->getClass()) {
      case kX86RegClassGp:
        ASMJIT_ASSERT(x86VarTypeToClass(ret.getVarType()) == vd->getClass());

        _context->unuse<kX86RegClassGp>(vd);
        _context->attach<kX86RegClassGp>(vd, regIndex, true);
        break;

      case kX86RegClassMm:
        ASMJIT_ASSERT(x86VarTypeToClass(ret.getVarType()) == vd->getClass());

        _context->unuse<kX86RegClassMm>(vd);
        _context->attach<kX86RegClassMm>(vd, regIndex, true);
        break;

      case kX86RegClassXyz:
        if (ret.getVarType() == kVarTypeFp32 || ret.getVarType() == kVarTypeFp64) {
          X86Mem m = _context->getVarMem(vd);
          m.setSize(
            (vf & VarInfo::kFlagSP) ? 4 :
            (vf & VarInfo::kFlagDP) ? 8 :
            (ret.getVarType() == kVarTypeFp32) ? 4 : 8);

          _context->unuse<kX86RegClassXyz>(vd, kVarStateMem);
          _compiler->fstp(m);
        }
        else {
          ASMJIT_ASSERT(x86VarTypeToClass(ret.getVarType()) == vd->getClass());

          _context->unuse<kX86RegClassXyz>(vd);
          _context->attach<kX86RegClassXyz>(vd, regIndex, true);
        }
        break;
    }
  }
}

// ============================================================================
// [asmjit::X86Context - TranslateOperands]
// ============================================================================

//! \internal
static Error X86Context_translateOperands(X86Context* self, Operand* opList, uint32_t opCount) {
  X86Compiler* compiler = self->getCompiler();
  uint32_t hasGpdBase = compiler->getRegSize() == 4;

  // Translate variables into registers.
  for (uint32_t i = 0; i < opCount; i++) {
    Operand* op = &opList[i];

    if (op->isVar()) {
      VarData* vd = compiler->getVdById(op->getId());
      ASMJIT_ASSERT(vd != nullptr);
      ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

      op->_vreg.op = Operand::kTypeReg;
      op->_vreg.index = vd->getRegIndex();
    }
    else if (op->isMem()) {
      X86Mem* m = static_cast<X86Mem*>(op);

      if (m->isBaseIndexType() && OperandUtil::isVarId(m->getBase())) {
        VarData* vd = compiler->getVdById(m->getBase());

        if (m->getMemType() == kMemTypeBaseIndex) {
          ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);
          op->_vmem.base = vd->getRegIndex();
        }
        else {
          if (!vd->isMemArg())
            self->getVarCell(vd);

          // Offset will be patched later by X86Context_patchFuncMem().
          m->setGpdBase(hasGpdBase);
          m->adjust(vd->isMemArg() ? self->_argActualDisp : self->_varActualDisp);
        }
      }

      if (OperandUtil::isVarId(m->getIndex())) {
        VarData* vd = compiler->getVdById(m->getIndex());
        ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);
        ASMJIT_ASSERT(vd->getRegIndex() != kX86RegIndexR12);
        op->_vmem.index = vd->getRegIndex();
      }
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Context - TranslatePrologEpilog]
// ============================================================================

//! \internal
static Error X86Context_initFunc(X86Context* self, X86FuncNode* func) {
  X86Compiler* compiler = self->getCompiler();
  X86FuncDecl* decl = func->getDecl();

  X86RegMask& clobberedRegs = self->_clobberedRegs;
  uint32_t regSize = compiler->getRegSize();

  // Setup "Save-Restore" registers.
  func->_saveRestoreRegs.set(kX86RegClassGp , clobberedRegs.get(kX86RegClassGp ) & decl->getPreserved(kX86RegClassGp ));
  func->_saveRestoreRegs.set(kX86RegClassMm , clobberedRegs.get(kX86RegClassMm ) & decl->getPreserved(kX86RegClassMm ));
  func->_saveRestoreRegs.set(kX86RegClassK  , 0);
  func->_saveRestoreRegs.set(kX86RegClassXyz, clobberedRegs.get(kX86RegClassXyz) & decl->getPreserved(kX86RegClassXyz));

  ASMJIT_ASSERT(!func->_saveRestoreRegs.has(kX86RegClassGp, Utils::mask(kX86RegIndexSp)));

  // Setup required stack alignment and kFuncFlagIsStackMisaligned.
  {
    uint32_t requiredStackAlignment = Utils::iMax(self->_memMaxAlign, self->getRegSize());

    if (requiredStackAlignment < 16) {
      // Require 16-byte alignment if 8-byte vars are used.
      if (self->_mem8ByteVarsUsed)
        requiredStackAlignment = 16;
      else if (func->_saveRestoreRegs.get(kX86RegClassMm) || func->_saveRestoreRegs.get(kX86RegClassXyz))
        requiredStackAlignment = 16;
      else if (Utils::inInterval<uint32_t>(func->getRequiredStackAlignment(), 8, 16))
        requiredStackAlignment = 16;
    }

    if (func->getRequiredStackAlignment() < requiredStackAlignment)
      func->setRequiredStackAlignment(requiredStackAlignment);

    func->updateRequiredStackAlignment();
  }

  // Adjust stack pointer if function is caller.
  if (func->isCaller()) {
    func->addFuncFlags(kFuncFlagIsStackAdjusted);
    func->_callStackSize = Utils::alignTo<uint32_t>(func->getCallStackSize(), func->getRequiredStackAlignment());
  }

  // Adjust stack pointer if manual stack alignment is needed.
  if (func->isStackMisaligned() && func->isNaked()) {
    // Get a memory cell where the original stack frame will be stored.
    VarCell* cell = self->_newStackCell(regSize, regSize);
    if (cell == nullptr)
      return self->getLastError(); // The error has already been set.

    func->addFuncFlags(kFuncFlagIsStackAdjusted);
    self->_stackFrameCell = cell;

    if (decl->getArgStackSize() > 0) {
      func->addFuncFlags(kFuncFlagX86MoveArgs);
      func->setExtraStackSize(decl->getArgStackSize());
    }

    // Get temporary register which will be used to align the stack frame.
    uint32_t fRegMask = Utils::bits(self->_regCount.getGp());
    uint32_t stackFrameCopyRegs;

    fRegMask &= ~(decl->getUsed(kX86RegClassGp) | Utils::mask(kX86RegIndexSp));
    stackFrameCopyRegs = fRegMask;

    // Try to remove modified registers from the mask.
    uint32_t tRegMask = fRegMask & ~self->getClobberedRegs(kX86RegClassGp);
    if (tRegMask != 0)
      fRegMask = tRegMask;

    // Try to remove preserved registers from the mask.
    tRegMask = fRegMask & ~decl->getPreserved(kX86RegClassGp);
    if (tRegMask != 0)
      fRegMask = tRegMask;

    ASMJIT_ASSERT(fRegMask != 0);

    uint32_t fRegIndex = Utils::findFirstBit(fRegMask);
    func->_stackFrameRegIndex = static_cast<uint8_t>(fRegIndex);

    // We have to save the register on the stack (it will be the part of prolog
    // and epilog), however we shouldn't save it twice, so we will remove it
    // from '_saveRestoreRegs' in case that it is preserved.
    fRegMask = Utils::mask(fRegIndex);
    if ((fRegMask & decl->getPreserved(kX86RegClassGp)) != 0) {
      func->_saveRestoreRegs.andNot(kX86RegClassGp, fRegMask);
      func->_isStackFrameRegPreserved = true;
    }

    if (func->hasFuncFlag(kFuncFlagX86MoveArgs)) {
      uint32_t maxRegs = (func->getArgStackSize() + regSize - 1) / regSize;
      stackFrameCopyRegs &= ~fRegMask;

      tRegMask = stackFrameCopyRegs & self->getClobberedRegs(kX86RegClassGp);
      uint32_t tRegCnt = Utils::bitCount(tRegMask);

      if (tRegCnt > 1 || (tRegCnt > 0 && tRegCnt <= maxRegs))
        stackFrameCopyRegs = tRegMask;
      else
        stackFrameCopyRegs = Utils::keepNOnesFromRight(stackFrameCopyRegs, Utils::iMin<uint32_t>(maxRegs, 2));

      func->_saveRestoreRegs.or_(kX86RegClassGp, stackFrameCopyRegs & decl->getPreserved(kX86RegClassGp));
      Utils::indexNOnesFromRight(func->_stackFrameCopyGpIndex, stackFrameCopyRegs, maxRegs);
    }
  }
  // If function is not naked we generate standard "EBP/RBP" stack frame.
  else if (!func->isNaked()) {
    uint32_t fRegIndex = kX86RegIndexBp;

    func->_stackFrameRegIndex = static_cast<uint8_t>(fRegIndex);
    func->_isStackFrameRegPreserved = true;
  }

  ASMJIT_PROPAGATE_ERROR(self->resolveCellOffsets());

  // Adjust stack pointer if requested memory can't fit into "Red Zone" or "Spill Zone".
  if (self->_memAllTotal > Utils::iMax<uint32_t>(func->getRedZoneSize(), func->getSpillZoneSize())) {
    func->addFuncFlags(kFuncFlagIsStackAdjusted);
  }

  // Setup stack size used to save preserved registers.
  {
    uint32_t memGpSize  = Utils::bitCount(func->_saveRestoreRegs.get(kX86RegClassGp )) * regSize;
    uint32_t memMmSize  = Utils::bitCount(func->_saveRestoreRegs.get(kX86RegClassMm )) * 8;
    uint32_t memXmmSize = Utils::bitCount(func->_saveRestoreRegs.get(kX86RegClassXyz)) * 16;

    func->_pushPopStackSize = memGpSize;
    func->_moveStackSize = memXmmSize + Utils::alignTo<uint32_t>(memMmSize, 16);
  }

  // Setup adjusted stack size.
  if (func->isStackMisaligned()) {
    func->_alignStackSize = 0;
  }
  else {
    // If function is aligned, the RETURN address is stored in the aligned
    // [ZSP - PtrSize] which makes current ZSP unaligned.
    int32_t v = static_cast<int32_t>(regSize);

    // If we have to store function frame pointer we have to count it as well,
    // because it is the first thing pushed on the stack.
    if (func->hasStackFrameReg() && func->isStackFrameRegPreserved())
      v += regSize;

    // Count push/pop sequence.
    v += func->getPushPopStackSize();

    // Count save/restore sequence for XMM registers (should be already aligned).
    v += func->getMoveStackSize();

    // Maximum memory required to call all functions within this function.
    v += func->getCallStackSize();

    // Calculate the final offset to keep stack alignment.
    func->_alignStackSize = Utils::alignDiff<uint32_t>(v, func->getRequiredStackAlignment());
  }

  // Memory stack size.
  func->_memStackSize = self->_memAllTotal;
  func->_alignedMemStackSize = Utils::alignTo<uint32_t>(func->_memStackSize, func->getRequiredStackAlignment());

  if (func->isNaked()) {
    self->_argBaseReg = kX86RegIndexSp;

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
    self->_argBaseReg = kX86RegIndexBp;
    // Caused by "push zbp".
    self->_argBaseOffset = regSize;
  }

  self->_varBaseReg = kX86RegIndexSp;
  self->_varBaseOffset = func->getCallStackSize();

  if (!func->isStackAdjusted()) {
    self->_varBaseOffset = -static_cast<int32_t>(
      func->_alignStackSize +
      func->_alignedMemStackSize +
      func->_moveStackSize);
  }

  return kErrorOk;
}

//! \internal
static Error X86Context_patchFuncMem(X86Context* self, X86FuncNode* func, HLNode* stop) {
  X86Compiler* compiler = self->getCompiler();
  HLNode* node = func;

  do {
    if (node->getType() == HLNode::kTypeInst) {
      HLInst* iNode = static_cast<HLInst*>(node);

      if (iNode->hasMemOp()) {
        X86Mem* m = iNode->getMemOp<X86Mem>();

        if (m->getMemType() == kMemTypeStackIndex && OperandUtil::isVarId(m->getBase())) {
          VarData* vd = compiler->getVdById(m->getBase());
          ASMJIT_ASSERT(vd != nullptr);

          if (vd->isMemArg()) {
            m->_vmem.base = self->_argBaseReg;
            m->_vmem.displacement += self->_argBaseOffset + vd->getMemOffset();
          }
          else {
            VarCell* cell = vd->getMemCell();
            ASMJIT_ASSERT(cell != nullptr);

            m->_vmem.base = self->_varBaseReg;
            m->_vmem.displacement += self->_varBaseOffset + cell->getOffset();
          }
        }
      }
    }

    node = node->getNext();
  } while (node != stop);

  return kErrorOk;
}

//! \internal
static Error X86Context_translatePrologEpilog(X86Context* self, X86FuncNode* func) {
  X86Compiler* compiler = self->getCompiler();
  X86FuncDecl* decl = func->getDecl();

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
  uint32_t regsGp  = func->getSaveRestoreRegs(kX86RegClassGp );
  uint32_t regsMm  = func->getSaveRestoreRegs(kX86RegClassMm );
  uint32_t regsXmm = func->getSaveRestoreRegs(kX86RegClassXyz);

  bool earlyPushPop = false;
  bool useLeaEpilog = false;

  X86GpReg gpReg(self->_zsp);
  X86GpReg fpReg(self->_zbp);

  X86Mem fpOffset;

  // --------------------------------------------------------------------------
  // [Prolog]
  // --------------------------------------------------------------------------

  compiler->_setCursor(func->getEntryNode());

  // Entry.
  if (func->isNaked()) {
    if (func->isStackMisaligned()) {
      fpReg.setIndex(func->getStackFrameRegIndex());
      fpOffset = x86::ptr(self->_zsp, self->_varBaseOffset + static_cast<int32_t>(self->_stackFrameCell->getOffset()));

      earlyPushPop = true;
      self->emitPushSequence(regsGp);

      if (func->isStackFrameRegPreserved())
        compiler->emit(kX86InstIdPush, fpReg);

      compiler->emit(kX86InstIdMov, fpReg, self->_zsp);
    }
  }
  else {
    compiler->emit(kX86InstIdPush, fpReg);
    compiler->emit(kX86InstIdMov, fpReg, self->_zsp);
  }

  if (!earlyPushPop) {
    self->emitPushSequence(regsGp);
    if (func->isStackMisaligned() && regsGp != 0)
      useLeaEpilog = true;
  }

  // Adjust stack pointer.
  if (func->isStackAdjusted()) {
    stackBase = static_cast<int32_t>(func->getAlignedMemStackSize() + func->getCallStackSize());

    if (stackSize)
      compiler->emit(kX86InstIdSub, self->_zsp, stackSize);

    if (func->isStackMisaligned())
      compiler->emit(kX86InstIdAnd, self->_zsp, -stackAlignment);

    if (func->isStackMisaligned() && func->isNaked())
      compiler->emit(kX86InstIdMov, fpOffset, fpReg);
  }
  else {
    stackBase = -static_cast<int32_t>(func->getAlignStackSize() + func->getMoveStackSize());
  }

  // Save Xmm/Mm/Gp (Mov).
  stackPtr = stackBase;
  for (i = 0, mask = regsXmm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kX86InstIdMovaps, x86::oword_ptr(self->_zsp, stackPtr), x86::xmm(i));
      stackPtr += 16;
    }
  }

  for (i = 0, mask = regsMm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kX86InstIdMovq, x86::qword_ptr(self->_zsp, stackPtr), x86::mm(i));
      stackPtr += 8;
    }
  }

  // --------------------------------------------------------------------------
  // [Move-Args]
  // --------------------------------------------------------------------------

  if (func->hasFuncFlag(kFuncFlagX86MoveArgs)) {
    uint32_t argStackPos = 0;
    uint32_t argStackSize = decl->getArgStackSize();

    uint32_t moveIndex = 0;
    uint32_t moveCount = (argStackSize + regSize - 1) / regSize;

    X86GpReg r[8];
    uint32_t numRegs = 0;

    for (i = 0; i < ASMJIT_ARRAY_SIZE(func->_stackFrameCopyGpIndex); i++)
      if (func->_stackFrameCopyGpIndex[i] != kInvalidReg)
        r[numRegs++] = gpReg.setIndex(func->_stackFrameCopyGpIndex[i]);
    ASMJIT_ASSERT(numRegs > 0);

    int32_t dSrc = func->getPushPopStackSize() + regSize;
    int32_t dDst = func->getAlignStackSize() +
                   func->getCallStackSize() +
                   func->getAlignedMemStackSize() +
                   func->getMoveStackSize();

    if (func->isStackFrameRegPreserved())
      dSrc += regSize;

    X86Mem mSrc = x86::ptr(fpReg, dSrc);
    X86Mem mDst = x86::ptr(self->_zsp, dDst);

    while (moveIndex < moveCount) {
      uint32_t numMovs = Utils::iMin<uint32_t>(moveCount - moveIndex, numRegs);

      for (i = 0; i < numMovs; i++)
        compiler->emit(kX86InstIdMov, r[i], mSrc.adjusted((moveIndex + i) * regSize));
      for (i = 0; i < numMovs; i++)
        compiler->emit(kX86InstIdMov, mDst.adjusted((moveIndex + i) * regSize), r[i]);

      argStackPos += numMovs * regSize;
      moveIndex += numMovs;
    }
  }

  // --------------------------------------------------------------------------
  // [Epilog]
  // --------------------------------------------------------------------------

  compiler->_setCursor(func->getExitNode());

  // Restore Xmm/Mm/Gp (Mov).
  stackPtr = stackBase;
  for (i = 0, mask = regsXmm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kX86InstIdMovaps, x86::xmm(i), x86::oword_ptr(self->_zsp, stackPtr));
      stackPtr += 16;
    }
  }

  for (i = 0, mask = regsMm; mask != 0; i++, mask >>= 1) {
    if (mask & 0x1) {
      compiler->emit(kX86InstIdMovq, x86::mm(i), x86::qword_ptr(self->_zsp, stackPtr));
      stackPtr += 8;
    }
  }

  // Adjust stack.
  if (useLeaEpilog) {
    compiler->emit(kX86InstIdLea, self->_zsp, x86::ptr(fpReg, -static_cast<int32_t>(func->getPushPopStackSize())));
  }
  else if (!func->isStackMisaligned()) {
    if (func->isStackAdjusted() && stackSize != 0)
      compiler->emit(kX86InstIdAdd, self->_zsp, stackSize);
  }

  // Restore Gp (Push/Pop).
  if (!earlyPushPop)
    self->emitPopSequence(regsGp);

  // Emms.
  if (func->hasFuncFlag(kFuncFlagX86Emms))
    compiler->emit(kX86InstIdEmms);

  // MFence/SFence/LFence.
  if (func->hasFuncFlag(kFuncFlagX86SFence) & func->hasFuncFlag(kFuncFlagX86LFence))
    compiler->emit(kX86InstIdMfence);
  else if (func->hasFuncFlag(kFuncFlagX86SFence))
    compiler->emit(kX86InstIdSfence);
  else if (func->hasFuncFlag(kFuncFlagX86LFence))
    compiler->emit(kX86InstIdLfence);

  // Leave.
  if (func->isNaked()) {
    if (func->isStackMisaligned()) {
      compiler->emit(kX86InstIdMov, self->_zsp, fpOffset);

      if (func->isStackFrameRegPreserved())
        compiler->emit(kX86InstIdPop, fpReg);

      if (earlyPushPop)
        self->emitPopSequence(regsGp);
    }
  }
  else {
    if (useLeaEpilog) {
      compiler->emit(kX86InstIdPop, fpReg);
    }
    else if (func->hasFuncFlag(kFuncFlagX86Leave)) {
      compiler->emit(kX86InstIdLeave);
    }
    else {
      compiler->emit(kX86InstIdMov, self->_zsp, fpReg);
      compiler->emit(kX86InstIdPop, fpReg);
    }
  }

  // Emit return.
  if (decl->getCalleePopsStack())
    compiler->emit(kX86InstIdRet, static_cast<int32_t>(decl->getArgStackSize()));
  else
    compiler->emit(kX86InstIdRet);

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Context - Translate - Jump]
// ============================================================================

//! \internal
static void X86Context_translateJump(X86Context* self, HLJump* jNode, HLLabel* jTarget) {
  X86Compiler* compiler = self->getCompiler();
  HLNode* extNode = self->getExtraBlock();

  compiler->_setCursor(extNode);
  self->switchState(jTarget->getState());

  // If one or more instruction has been added during switchState() it will be
  // moved at the end of the function body.
  if (compiler->getCursor() != extNode) {
    // TODO: Can fail.
    HLLabel* jTrampolineTarget = compiler->newLabelNode();

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

  // Store the `extNode` and load the state back.
  self->setExtraBlock(extNode);
  self->loadState(jNode->_state);
}

// ============================================================================
// [asmjit::X86Context - Translate - Ret]
// ============================================================================

static Error X86Context_translateRet(X86Context* self, HLRet* rNode, HLLabel* exitTarget) {
  X86Compiler* compiler = self->getCompiler();
  HLNode* node = rNode->getNext();

  // 32-bit mode requires to push floating point return value(s), handle it
  // here as it's a special case.
  X86VarMap* map = rNode->getMap<X86VarMap>();
  if (map != nullptr) {
    VarAttr* vaList = map->getVaList();
    uint32_t vaCount = map->getVaCount();

    for (uint32_t i = 0; i < vaCount; i++) {
      VarAttr& va = vaList[i];
      if (va.hasFlag(kVarAttrX86Fld4 | kVarAttrX86Fld8)) {
        VarData* vd = va.getVd();
        X86Mem m(self->getVarMem(vd));

        uint32_t flags = _x86VarInfo[vd->getType()].getFlags();
        m.setSize(
          (flags & VarInfo::kFlagSP) ? 4 :
          (flags & VarInfo::kFlagDP) ? 8 :
          va.hasFlag(kVarAttrX86Fld4) ? 4 : 8);

        compiler->fld(m);
      }
    }
  }

  // Decide whether to `jmp` or not in case we are next to the return label.
  while (node != nullptr) {
    switch (node->getType()) {
      // If we have found an exit label we just return, there is no need to
      // emit jump to that.
      case HLNode::kTypeLabel:
        if (static_cast<HLLabel*>(node) == exitTarget)
          return kErrorOk;
        goto _EmitRet;

      case HLNode::kTypeData:
      case HLNode::kTypeInst:
      case HLNode::kTypeCall:
      case HLNode::kTypeRet:
        goto _EmitRet;

      // Continue iterating.
      case HLNode::kTypeComment:
      case HLNode::kTypeAlign:
      case HLNode::kTypeHint:
        break;

      // Invalid node to be here.
      case HLNode::kTypeFunc:
        return self->getCompiler()->setLastError(kErrorInvalidState);

      // We can't go forward from here.
      case HLNode::kTypeSentinel:
        return kErrorOk;
    }

    node = node->getNext();
  }

_EmitRet:
  {
    compiler->_setCursor(rNode);
    compiler->jmp(exitTarget->getLabel());
  }
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Context - Translate - Func]
// ============================================================================

Error X86Context::translate() {
  ASMJIT_TLOG("[T] ======= Translate (Begin)\n");

  X86Compiler* compiler = getCompiler();
  X86FuncNode* func = getFunc();

  // Register allocator contexts.
  X86VarAlloc vAlloc(this);
  X86CallAlloc cAlloc(this);

  // Flow.
  HLNode* node_ = func;
  HLNode* next = nullptr;
  HLNode* stop = getStop();

  PodList<HLNode*>::Link* jLink = _jccList.getFirst();

  for (;;) {
    while (node_->isTranslated()) {
      // Switch state if we went to the already translated node.
      if (node_->getType() == HLNode::kTypeLabel) {
        HLLabel* node = static_cast<HLLabel*>(node_);
        compiler->_setCursor(node->getPrev());
        switchState(node->getState());
      }

_NextGroup:
      if (jLink == nullptr) {
        goto _Done;
      }
      else {
        node_ = jLink->getValue();
        jLink = jLink->getNext();

        HLNode* jFlow = X86Context_getOppositeJccFlow(static_cast<HLJump*>(node_));
        loadState(node_->getState());

        if (jFlow->getState()) {
          X86Context_translateJump(this,
            static_cast<HLJump*>(node_),
            static_cast<HLLabel*>(jFlow));

          node_ = jFlow;
          if (node_->isTranslated())
            goto _NextGroup;
        }
        else {
          node_ = jFlow;
        }

        break;
      }
    }

    next = node_->getNext();
    node_->orFlags(HLNode::kFlagIsTranslated);

    ASMJIT_TSEC({
      this->_traceNode(this, node_, "[T] ");
    });

    switch (node_->getType()) {
      // ----------------------------------------------------------------------
      // [Align / Embed]
      // ----------------------------------------------------------------------

      case HLNode::kTypeAlign:
      case HLNode::kTypeData:
        break;

      // ----------------------------------------------------------------------
      // [Target]
      // ----------------------------------------------------------------------

      case HLNode::kTypeLabel: {
        HLLabel* node = static_cast<HLLabel*>(node_);
        ASMJIT_ASSERT(!node->hasState());
        node->setState(saveState());
        break;
      }

      // ----------------------------------------------------------------------
      // [Inst/Call/SArg/Ret]
      // ----------------------------------------------------------------------

      case HLNode::kTypeInst:
      case HLNode::kTypeCall:
      case HLNode::kTypeCallArg:
        // Update VarAttr's unuse flags based on liveness of the next node.
        if (!node_->isJcc()) {
          X86VarMap* map = static_cast<X86VarMap*>(node_->getMap());
          BitArray* liveness;

          if (map != nullptr && next != nullptr && (liveness = next->getLiveness()) != nullptr) {
            VarAttr* vaList = map->getVaList();
            uint32_t vaCount = map->getVaCount();

            for (uint32_t i = 0; i < vaCount; i++) {
              VarAttr* va = &vaList[i];
              VarData* vd = va->getVd();

              if (!liveness->getBit(vd->getLocalId()))
                va->orFlags(kVarAttrUnuse);
            }
          }
        }

        if (node_->getType() == HLNode::kTypeCall) {
          ASMJIT_PROPAGATE_ERROR(cAlloc.run(static_cast<X86CallNode*>(node_)));
          break;
        }
        ASMJIT_FALLTHROUGH;

      case HLNode::kTypeHint:
      case HLNode::kTypeRet: {
        ASMJIT_PROPAGATE_ERROR(vAlloc.run(node_));

        // Handle conditional/unconditional jump.
        if (node_->isJmpOrJcc()) {
          HLJump* node = static_cast<HLJump*>(node_);
          HLLabel* jTarget = node->getTarget();

          // Target not followed.
          if (jTarget == nullptr) {
            if (node->isJmp())
              goto _NextGroup;
            else
              break;
          }

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
            HLNode* jNext = node->getNext();

            if (jTarget->isTranslated()) {
              if (jNext->isTranslated()) {
                ASMJIT_ASSERT(jNext->getType() == HLNode::kTypeLabel);
                compiler->_setCursor(node->getPrev());
                intersectStates(jTarget->getState(), jNext->getState());
              }

              VarState* savedState = saveState();
              node->setState(savedState);

              X86Context_translateJump(this, node, jTarget);
              next = jNext;
            }
            else if (jNext->isTranslated()) {
              ASMJIT_ASSERT(jNext->getType() == HLNode::kTypeLabel);

              VarState* savedState = saveState();
              node->setState(savedState);

              compiler->_setCursor(node);
              switchState(static_cast<HLLabel*>(jNext)->getState());
              next = jTarget;
            }
            else {
              node->setState(saveState());
              next = X86Context_getJccFlow(node);
            }
          }
        }
        else if (node_->isRet()) {
          ASMJIT_PROPAGATE_ERROR(
            X86Context_translateRet(this, static_cast<HLRet*>(node_), func->getExitNode()));
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Func]
      // ----------------------------------------------------------------------

      case HLNode::kTypeFunc: {
        ASMJIT_ASSERT(node_ == func);

        X86FuncDecl* decl = func->getDecl();
        X86VarMap* map = func->getMap<X86VarMap>();

        if (map != nullptr) {
          uint32_t i;
          uint32_t argCount = func->_x86Decl.getNumArgs();

          for (i = 0; i < argCount; i++) {
            const FuncInOut& arg = decl->getArg(i);

            VarData* vd = func->getArg(i);
            if (vd == nullptr)
              continue;

            VarAttr* va = map->findVa(vd);
            ASMJIT_ASSERT(va != nullptr);

            if (va->getFlags() & kVarAttrUnuse)
              continue;

            uint32_t regIndex = va->getOutRegIndex();
            if (regIndex != kInvalidReg && (va->getFlags() & kVarAttrWConv) == 0) {
              switch (vd->getClass()) {
                case kX86RegClassGp : attach<kX86RegClassGp >(vd, regIndex, true); break;
                case kX86RegClassMm : attach<kX86RegClassMm >(vd, regIndex, true); break;
                case kX86RegClassXyz: attach<kX86RegClassXyz>(vd, regIndex, true); break;
              }
            }
            else if (va->hasFlag(kVarAttrWConv)) {
              // TODO: [COMPILER] Function Argument Conversion.
              ASMJIT_NOT_REACHED();
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

      case HLNode::kTypeSentinel: {
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
  ASMJIT_PROPAGATE_ERROR(X86Context_initFunc(this, func));
  ASMJIT_PROPAGATE_ERROR(X86Context_patchFuncMem(this, func, stop));
  ASMJIT_PROPAGATE_ERROR(X86Context_translatePrologEpilog(this, func));

  ASMJIT_TLOG("[T] ======= Translate (End)\n");
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Context - Serialize]
// ============================================================================

Error X86Context::serialize(Assembler* assembler_, HLNode* start, HLNode* stop) {
  X86Assembler* assembler = static_cast<X86Assembler*>(assembler_);
  HLNode* node_ = start;

#if !defined(ASMJIT_DISABLE_LOGGER)
  Logger* logger = assembler->getLogger();
#endif // !ASMJIT_DISABLE_LOGGER

  do {
#if !defined(ASMJIT_DISABLE_LOGGER)
    if (logger) {
      _stringBuilder.clear();
      formatInlineComment(_stringBuilder, node_);
      assembler->_comment = _stringBuilder.getData();
    }
#endif // !ASMJIT_DISABLE_LOGGER

    switch (node_->getType()) {
      case HLNode::kTypeAlign: {
        HLAlign* node = static_cast<HLAlign*>(node_);
        assembler->align(node->getAlignMode(), node->getOffset());
        break;
      }

      case HLNode::kTypeData: {
        HLData* node = static_cast<HLData*>(node_);
        assembler->embed(node->getData(), node->getSize());
        break;
      }

      case HLNode::kTypeComment: {
#if !defined(ASMJIT_DISABLE_LOGGER)
        HLComment* node = static_cast<HLComment*>(node_);
        if (logger)
          logger->logFormat(Logger::kStyleComment,
            "%s; %s\n", logger->getIndentation(), node->getComment());
#endif // !ASMJIT_DISABLE_LOGGER
        break;
      }

      case HLNode::kTypeHint: {
        break;
      }

      case HLNode::kTypeLabel: {
        HLLabel* node = static_cast<HLLabel*>(node_);
        assembler->bind(node->getLabel());
        break;
      }

      case HLNode::kTypeInst: {
        HLInst* node = static_cast<HLInst*>(node_);

        uint32_t instId = node->getInstId();
        uint32_t opCount = node->getOpCount();

        const Operand* opList = node->getOpList();
        assembler->_instOptions = node->getOptions();

        const Operand* o0 = &noOperand;
        const Operand* o1 = &noOperand;
        const Operand* o2 = &noOperand;
        const Operand* o3 = &noOperand;

        if (node->isSpecial()) {
          switch (instId) {
            case kX86InstIdCpuid:
              break;

            case kX86InstIdCbw:
            case kX86InstIdCdq:
            case kX86InstIdCdqe:
            case kX86InstIdCwd:
            case kX86InstIdCwde:
            case kX86InstIdCqo:
              break;

            case kX86InstIdCmpxchg:
              o0 = &opList[1];
              o1 = &opList[2];
              break;

            case kX86InstIdCmpxchg8b:
            case kX86InstIdCmpxchg16b:
              o0 = &opList[4];
              break;

            case kX86InstIdDaa:
            case kX86InstIdDas:
              break;

            case kX86InstIdImul:
            case kX86InstIdMul:
            case kX86InstIdIdiv:
            case kX86InstIdDiv:
              // Assume "Mul/Div dst_hi (implicit), dst_lo (implicit), src (explicit)".
              ASMJIT_ASSERT(opCount == 3);
              o0 = &opList[2];
              break;

            case kX86InstIdMovPtr:
              break;

            case kX86InstIdLahf:
            case kX86InstIdSahf:
              break;

            case kX86InstIdMaskmovq:
            case kX86InstIdMaskmovdqu:
              o0 = &opList[1];
              o1 = &opList[2];
              break;

            case kX86InstIdEnter:
              o0 = &opList[0];
              o1 = &opList[1];
              break;

            case kX86InstIdLeave:
              break;

            case kX86InstIdRet:
              if (opCount > 0)
                o0 = &opList[0];
              break;

            case kX86InstIdMonitor:
            case kX86InstIdMwait:
              break;

            case kX86InstIdPop:
              o0 = &opList[0];
              break;

            case kX86InstIdPopa:
            case kX86InstIdPopf:
              break;

            case kX86InstIdPush:
              o0 = &opList[0];
              break;

            case kX86InstIdPusha:
            case kX86InstIdPushf:
              break;

            case kX86InstIdRcl:
            case kX86InstIdRcr:
            case kX86InstIdRol:
            case kX86InstIdRor:
            case kX86InstIdSal:
            case kX86InstIdSar:
            case kX86InstIdShl:
            case kX86InstIdShr:
              o0 = &opList[0];
              o1 = &x86::cl;
              break;

            case kX86InstIdShld:
            case kX86InstIdShrd:
              o0 = &opList[0];
              o1 = &opList[1];
              o2 = &x86::cl;
              break;

            case kX86InstIdRdtsc:
            case kX86InstIdRdtscp:
              break;

            case kX86InstIdRepLodsB: case kX86InstIdRepLodsD: case kX86InstIdRepLodsQ: case kX86InstIdRepLodsW:
            case kX86InstIdRepMovsB: case kX86InstIdRepMovsD: case kX86InstIdRepMovsQ: case kX86InstIdRepMovsW:
            case kX86InstIdRepStosB: case kX86InstIdRepStosD: case kX86InstIdRepStosQ: case kX86InstIdRepStosW:
            case kX86InstIdRepeCmpsB: case kX86InstIdRepeCmpsD: case kX86InstIdRepeCmpsQ: case kX86InstIdRepeCmpsW:
            case kX86InstIdRepeScasB: case kX86InstIdRepeScasD: case kX86InstIdRepeScasQ: case kX86InstIdRepeScasW:
            case kX86InstIdRepneCmpsB: case kX86InstIdRepneCmpsD: case kX86InstIdRepneCmpsQ: case kX86InstIdRepneCmpsW:
            case kX86InstIdRepneScasB: case kX86InstIdRepneScasD: case kX86InstIdRepneScasQ: case kX86InstIdRepneScasW:
              break;

            case kX86InstIdXrstor:
            case kX86InstIdXrstor64:
            case kX86InstIdXsave:
            case kX86InstIdXsave64:
            case kX86InstIdXsaveopt:
            case kX86InstIdXsaveopt64:
              o0 = &opList[0];
              break;

            case kX86InstIdXgetbv:
            case kX86InstIdXsetbv:
              break;

            default:
              ASMJIT_NOT_REACHED();
          }
        }
        else {
          if (opCount > 0) o0 = &opList[0];
          if (opCount > 1) o1 = &opList[1];
          if (opCount > 2) o2 = &opList[2];
          if (opCount > 3) o3 = &opList[3];
        }

        // Should call _emit() directly as 4 operand form is the main form.
        assembler->emit(instId, *o0, *o1, *o2, *o3);
        break;
      }

      // Function scope and return is translated to another nodes, no special
      // handling is required at this point.
      case HLNode::kTypeFunc:
      case HLNode::kTypeSentinel:
      case HLNode::kTypeRet: {
        break;
      }

      // Function call adds nodes before and after, but it's required to emit
      // the call instruction by itself.
      case HLNode::kTypeCall: {
        X86CallNode* node = static_cast<X86CallNode*>(node_);
        assembler->emit(kX86InstIdCall, node->_target, noOperand, noOperand);
        break;
      }

      default:
        break;
    }

    node_ = node_->getNext();
  } while (node_ != stop);

  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER && (ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64)
