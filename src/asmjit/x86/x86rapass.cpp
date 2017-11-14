// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../core/cpuinfo.h"
#include "../core/intutils.h"
#include "../core/type.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86internal_p.h"
#include "../x86/x86rapass_p.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::X86RAPass - Helpers]
// ============================================================================

static ASMJIT_FORCEINLINE uint64_t immMaskFromSize(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static const uint64_t masks[] = {
    0x00000000000000FFU, //   1
    0x000000000000FFFFU, //   2
    0x00000000FFFFFFFFU, //   4
    0xFFFFFFFFFFFFFFFFU, //   8
    0x0000000000000000U, //  16
    0x0000000000000000U, //  32
    0x0000000000000000U, //  64
    0x0000000000000000U, // 128
    0x0000000000000000U  // 256
  };
  return masks[IntUtils::ctz(size)];
}

// ============================================================================
// [asmjit::X86OpInfo]
// ============================================================================

namespace X86OpInfo {
  enum {
    Any = Reg::kIdBad,
    Zax = X86Gp::kIdAx,
    Zbx = X86Gp::kIdBx,
    Zcx = X86Gp::kIdCx,
    Zdx = X86Gp::kIdDx,
    Zsi = X86Gp::kIdSi,
    Zdi = X86Gp::kIdDi
  };

  #define R(IDX) { uint16_t(OpInfo::kUse | OpInfo::kRead ), uint8_t(IDX), uint8_t(0) }
  #define W(IDX) { uint16_t(OpInfo::kOut | OpInfo::kWrite), uint8_t(IDX), uint8_t(0) }
  #define X(IDX) { uint16_t(OpInfo::kUse | OpInfo::kRW   ), uint8_t(IDX), uint8_t(0) }
  #define NONE() { uint16_t(0), uint8_t(Any), uint8_t(0) }
  #define DEFINE_OPS(NAME, ...) static const OpInfo NAME[6] = { __VA_ARGS__ }
  #define RETURN_OPS(...) do { DEFINE_OPS(ops, __VA_ARGS__); return ops; } while(0)

  // Common cases.
  DEFINE_OPS(op_r    , R(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_w    , W(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_x    , X(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_xx   , X(Any), X(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_w_all, W(Any), W(Any), W(Any), W(Any), W(Any), W(Any));

  static ASMJIT_FORCEINLINE const OpInfo* get(uint32_t instId, const X86Inst& instData, const Operand* opArray, uint32_t opCount) noexcept {
    const X86Inst::CommonData& commonData = instData.getCommonData();
    if (!commonData.hasFixedRM()) {
      if (commonData.isUseXX()) return op_xx;
      if (commonData.isUseX()) return op_x;
      if (commonData.isUseW()) return op_w;
      if (commonData.isUseR()) return op_r;
    }
    else {
      switch (instId) {
        case X86Inst::kIdAaa:
        case X86Inst::kIdAad:
        case X86Inst::kIdAam:
        case X86Inst::kIdAas:
        case X86Inst::kIdDaa:
        case X86Inst::kIdDas:
          RETURN_OPS(X(Zax));

        case X86Inst::kIdCpuid:
          RETURN_OPS(X(Zax), W(Zbx), X(Zcx), W(Zdx));

        case X86Inst::kIdCbw:
        case X86Inst::kIdCdqe:
        case X86Inst::kIdCwde:
          RETURN_OPS(X(Zax));

        case X86Inst::kIdCdq:
        case X86Inst::kIdCwd:
        case X86Inst::kIdCqo:
          RETURN_OPS(W(Zdx), R(Zax));

        case X86Inst::kIdCmpxchg:
          RETURN_OPS(X(Any), R(Any), X(Zax));

        case X86Inst::kIdCmpxchg8b:
        case X86Inst::kIdCmpxchg16b:
          RETURN_OPS(NONE(), X(Zdx), X(Zax), R(Zcx), R(Zbx));

        case X86Inst::kIdDiv:
        case X86Inst::kIdIdiv:
          if (opCount == 2)
            RETURN_OPS(X(Zax), R(Any));
          else
            RETURN_OPS(X(Zdx), X(Zax), R(Any));

        case X86Inst::kIdImul:
          if (opCount == 2) {
            if (X86Reg::isGpw(opArray[0]) && opArray[1].getSize() == 1)
              RETURN_OPS(W(Zax), R(Any)); // imul ax, r8/m8
            else
              RETURN_OPS(X(Any), R(Any)); // imul r?, r?/m?
          }

          if (opCount == 3) {
            if (opArray[2].isImm())
              return op_w;
            else
              RETURN_OPS(W(Zdx), X(Zax), R(Any));
          }
          break;

        case X86Inst::kIdMul:
          if (opCount == 2)
            RETURN_OPS(X(Zax), R(Any));
          else
            RETURN_OPS(W(Zdx), X(Zax), R(Any));

        case X86Inst::kIdMulx:
          RETURN_OPS(W(Any), W(Any), R(Any), R(Zdx));

        case X86Inst::kIdJecxz:
        case X86Inst::kIdLoop:
        case X86Inst::kIdLoope:
        case X86Inst::kIdLoopne:
          RETURN_OPS(R(Zcx));

        case X86Inst::kIdLahf: RETURN_OPS(W(Zax));
        case X86Inst::kIdSahf: RETURN_OPS(R(Zax));

        case X86Inst::kIdRet: break;
        case X86Inst::kIdEnter: break;
        case X86Inst::kIdLeave: break;

        case X86Inst::kIdMonitor    : RETURN_OPS(R(Zax), R(Zcx), R(Zdx));
        case X86Inst::kIdMwait      : RETURN_OPS(R(Zax), R(Zcx));

        case X86Inst::kIdPush       : return op_r;
        case X86Inst::kIdPop        : return op_w;

        case X86Inst::kIdRcl:
        case X86Inst::kIdRcr:
        case X86Inst::kIdRol:
        case X86Inst::kIdRor:
        case X86Inst::kIdSal:
        case X86Inst::kIdSar:
        case X86Inst::kIdShl:
        case X86Inst::kIdShr:
          RETURN_OPS(X(Any), R(Zcx));

        case X86Inst::kIdShld:
        case X86Inst::kIdShrd:
          RETURN_OPS(X(Any), R(Any), R(Zcx));

        case X86Inst::kIdRdtsc:
        case X86Inst::kIdRdtscp:
          RETURN_OPS(W(Zdx), W(Zax), W(Zcx));

        case X86Inst::kIdXrstor:
        case X86Inst::kIdXrstor64:
        case X86Inst::kIdXsave:
        case X86Inst::kIdXsave64:
        case X86Inst::kIdXsaveopt:
        case X86Inst::kIdXsaveopt64:
          RETURN_OPS(W(Any), R(Zdx), R(Zax));

        case X86Inst::kIdXgetbv:
          RETURN_OPS(W(Zdx), W(Zax), R(Zcx));

        case X86Inst::kIdXsetbv:
          RETURN_OPS(R(Zdx), R(Zax), R(Zcx));

        case X86Inst::kIdIn  : RETURN_OPS(W(Zax), R(Zdx));
        case X86Inst::kIdIns : RETURN_OPS(X(Zdi), R(Zdx));
        case X86Inst::kIdOut : RETURN_OPS(R(Zdx), R(Zax));
        case X86Inst::kIdOuts: RETURN_OPS(R(Zdx), X(Zsi));

        case X86Inst::kIdCmps: RETURN_OPS(X(Zsi), X(Zdi));
        case X86Inst::kIdLods: RETURN_OPS(W(Zax), X(Zsi));
        case X86Inst::kIdMovs: RETURN_OPS(X(Zdi), X(Zsi));
        case X86Inst::kIdScas: RETURN_OPS(X(Zdi), R(Zax));
        case X86Inst::kIdStos: RETURN_OPS(X(Zdi), R(Zax));

        case X86Inst::kIdMaskmovq:
        case X86Inst::kIdMaskmovdqu:
        case X86Inst::kIdVmaskmovdqu:
          RETURN_OPS(R(Any), R(Any), R(Zdi));

        case X86Inst::kIdBlendvpd:
        case X86Inst::kIdBlendvps:
        case X86Inst::kIdPblendvb:
        case X86Inst::kIdSha256rnds2:
          RETURN_OPS(W(Any), R(Any), R(0));

        case X86Inst::kIdPcmpestri  :
        case X86Inst::kIdVpcmpestri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx));
        case X86Inst::kIdPcmpistri  :
        case X86Inst::kIdVpcmpistri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx), R(Zax), R(Zdx));
        case X86Inst::kIdPcmpestrm  :
        case X86Inst::kIdVpcmpestrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0));
        case X86Inst::kIdPcmpistrm  :
        case X86Inst::kIdVpcmpistrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0)  , R(Zax), R(Zdx));
      }
    }

    return op_x;
  }

  #undef RETURN_OPS
  #undef DEFINE_OPS
  #undef NONE
  #undef X
  #undef W
  #undef R
} // X86OpInfo namespace

// ============================================================================
// [asmjit::X86RACFGBuilder]
// ============================================================================

class X86RACFGBuilder : public RACFGBuilder<X86RACFGBuilder> {
public:
  inline X86RACFGBuilder(X86RAPass* pass) noexcept
    : RACFGBuilder<X86RACFGBuilder>(pass),
      _is64Bit(pass->getGpSize() == 8) {}

  inline X86Compiler* cc() const noexcept { return static_cast<X86Compiler*>(_cc); }

  Error newInst(CBInst** out, uint32_t instId, const OpInfo* opInfo, const uint32_t* physRegs, const Operand_& o0, const Operand_& o1) noexcept;
  Error onInst(CBInst* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept;
  Error onCall(CCFuncCall* call, RAInstBuilder& ib) noexcept;
  Error moveImmToRegArg(CCFuncCall* call, const FuncValue& arg, const Imm& imm_, Reg* out) noexcept;
  Error moveImmToStackArg(CCFuncCall* call, const FuncValue& arg, const Imm& imm_) noexcept;
  Error moveRegToStackArg(CCFuncCall* call, const FuncValue& arg, const Reg& reg) noexcept;
  Error onRet(CCFuncRet* funcRet, RAInstBuilder& ib) noexcept;

  bool _is64Bit;
};

// ============================================================================
// [asmjit::X86RACFGBuilder - Utilities]
// ============================================================================

Error X86RACFGBuilder::newInst(CBInst** out, uint32_t instId, const OpInfo* opInfo, const uint32_t* physRegs, const Operand_& o0, const Operand_& o1) noexcept {
  CBInst* inst = cc()->newInstNode(instId, 0, o0, o1);
  if (ASMJIT_UNLIKELY(!inst))
    return DebugUtils::errored(kErrorNoHeapMemory);

  RAInstBuilder ib;
  uint32_t opCount = inst->getOpCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = inst->getOp(i);
    if (op.isReg()) {
      // Register operand.
      const X86Reg& reg = op.as<X86Reg>();
      uint32_t flags = opInfo[i].getFlags();

      uint32_t vIndex = Operand::unpackId(reg.getId());
      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->getGroup();
        uint32_t allocable = _pass->_availableRegs[group];

        uint32_t useId = Reg::kIdBad;
        uint32_t outId = Reg::kIdBad;

        uint32_t useRewriteMask = 0;
        uint32_t outRewriteMask = 0;

        if (opInfo[i].isUse()) {
          useId = physRegs[i];
          useRewriteMask = IntUtils::mask(inst->getRewriteIndex(&reg._reg.id));
        }
        else {
          outId = physRegs[i];
          outRewriteMask = IntUtils::mask(inst->getRewriteIndex(&reg._reg.id));
        }

        ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));
      }
    }
  }
  ASMJIT_PROPAGATE(_pass->assignRAInst(inst, _curBlock, ib));

  *out = inst;
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RACFGBuilder - OnInst]
// ============================================================================

Error X86RACFGBuilder::onInst(CBInst* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept {
  uint32_t instId = inst->getInstId();
  uint32_t opCount = inst->getOpCount();

  const OpInfo* opInfo = nullptr;

  if (X86Inst::isDefinedId(instId)) {
    const X86Inst& instData = X86Inst::getInst(instId);
    const X86Inst::CommonData& commonData = instData.getCommonData();

    bool hasGpbHiConstraint = false;
    uint32_t singleRegOps = 0;

    if (opCount) {
      const Operand* opArray = inst->getOpArray();
      opInfo = X86OpInfo::get(instId, instData, opArray, opCount);

      for (uint32_t i = 0; i < opCount; i++) {
        const Operand& op = opArray[i];
        if (op.isReg()) {
          // Register operand.
          const X86Reg& reg = op.as<X86Reg>();
          uint32_t flags = opInfo[i].getFlags();
          uint32_t allowedRegs = 0xFFFFFFFFU;

          // X86-specific constraints related to LO|HI general purpose registers.
          if (reg.isGpb()) {
            flags |= RATiedReg::kX86Gpb;
            if (!_is64Bit) {
              // Restrict to first four - AL|AH|BL|BH|CL|CH|DL|DH. In 32-bit mode
              // it's not possible to access SIL|DIL, etc, so this is just enough.
              allowedRegs = 0x0FU;
            }
            else {
              // If we encountered GPB-HI register the situation is much more
              // complicated than in 32-bit mode. We need to patch all registers
              // to not use ID higher than 7 and all GPB-LO registers to not use
              // index higher than 3. Instead of doing the patching here we just
              // set a flag and will do it later, to not complicate this loop.
              if (reg.isGpbHi()) {
                hasGpbHiConstraint = true;
                allowedRegs = 0x0FU;
              }
            }
          }

          uint32_t vIndex = Operand::unpackId(reg.getId());
          if (vIndex < Operand::kPackedIdCount) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

            uint32_t group = workReg->getGroup();
            uint32_t allocable = _pass->_availableRegs[group] & allowedRegs;

            uint32_t useId = Reg::kIdBad;
            uint32_t outId = Reg::kIdBad;

            uint32_t useRewriteMask = 0;
            uint32_t outRewriteMask = 0;

            if (opInfo[i].isUse()) {
              useId = opInfo[i].getPhysId();
              useRewriteMask = IntUtils::mask(inst->getRewriteIndex(&reg._reg.id));
            }
            else {
              outId = opInfo[i].getPhysId();
              outRewriteMask = IntUtils::mask(inst->getRewriteIndex(&reg._reg.id));
            }

            ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));

            if (singleRegOps == i)
              singleRegOps++;
          }
        }
        else if (op.isMem()) {
          // Memory operand.
          const X86Mem& mem = op.as<X86Mem>();
          if (mem.isRegHome()) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(mem.getBaseId()), &workReg));
            _pass->getOrCreateStackSlot(workReg);
          }
          else if (mem.hasBaseReg()) {
            uint32_t vIndex = Operand::unpackId(mem.getBaseId());
            if (vIndex < Operand::kPackedIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t group = workReg->getGroup();
              uint32_t allocable = _pass->_availableRegs[group];

              uint32_t useId = Reg::kIdBad;
              uint32_t outId = Reg::kIdBad;

              uint32_t useRewriteMask = 0;
              uint32_t outRewriteMask = 0;

              useId = opInfo[i].getPhysId();
              useRewriteMask = IntUtils::mask(inst->getRewriteIndex(&mem._reg.id));

              uint32_t flags = useId != Reg::kIdBad ? uint32_t(opInfo[i].getFlags()) : uint32_t(RATiedReg::kUse | RATiedReg::kRead);
              ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));
            }
          }

          if (mem.hasIndexReg()) {
            uint32_t vIndex = Operand::unpackId(mem.getIndexId());
            if (vIndex < Operand::kPackedIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t group = workReg->getGroup();
              uint32_t allocable = _pass->_availableRegs[group];
              uint32_t rewriteMask = IntUtils::mask(inst->getRewriteIndex(&mem._mem.index));

              ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, Reg::kIdBad, rewriteMask, Reg::kIdBad, 0));
            }
          }
        }
      }
    }

    // Handle extra operand (either REP {cx|ecx|rcx} or AVX-512 {k} selector).
    if (inst->hasExtraReg()) {
      uint32_t vIndex = Operand::unpackId(inst->getExtraReg().getId());
      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->getGroup();
        uint32_t rewriteMask = IntUtils::mask(inst->getRewriteIndex(&inst->getExtraReg()._id));

        if (group == X86Gp::kGroupK) {
          // AVX-512 mask selector {k} register - read-only, allocable to any register except {k0}.
          uint32_t allocableRegs= _pass->_availableRegs[group] & ~IntUtils::mask(0);
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocableRegs, Reg::kIdBad, rewriteMask, Reg::kIdBad, 0));
          singleRegOps = 0;
        }
        else {
          // REP {cx|ecx|rcx} register - read & write, allocable to {cx|ecx|rcx} only.
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRW, 0, X86Gp::kIdCx, rewriteMask, X86Gp::kIdBad, 0));
        }
      }
      else {
        uint32_t group = inst->getExtraReg().getGroup();
        if (group == X86Gp::kGroupK && inst->getExtraReg().getId() != 0)
          singleRegOps = 0;
      }
    }

    // Handle X86 constraints.
    if (hasGpbHiConstraint) {
      for (uint32_t i = 0; i < ib.getTiedRegCount(); i++) {
        RATiedReg* tiedReg = ib[i];
        tiedReg->_allocableRegs &= tiedReg->hasFlag(RATiedReg::kX86Gpb) ? 0x0FU : 0xFFU;
      }
    }

    if (ib.getTiedRegCount() == 1) {
      // Handle special cases of some instructions where all operands share the same
      // register. In such case the single operand becomes read-only or write-only.
      uint32_t singleRegCase = X86Inst::kSingleRegNone;
      if (singleRegOps == opCount) {
        singleRegCase = commonData.getSingleRegCase();
      }
      else if (opCount == 2 && inst->getOp(1).isImm()) {
        // Handle some tricks used by X86 asm.
        const Reg& reg = inst->getOp(0).as<Reg>();
        const Imm& imm = inst->getOp(1).as<Imm>();

        const RAWorkReg* workReg = _pass->getWorkReg(ib[0]->getWorkId());
        uint32_t workRegSize = workReg->getInfo().getSize();

        switch (inst->getInstId()) {
          case X86Inst::kIdOr: {
            // Sets the value of the destination register to -1, previous content unused.
            if (reg.getSize() >= 4 || reg.getSize() >= workRegSize) {
              if (imm.getInt64() == -1 || imm.getUInt64() == immMaskFromSize(reg.getSize()))
                singleRegCase = X86Inst::kSingleRegWO;
            }
            ASMJIT_FALLTHROUGH;
          }

          case X86Inst::kIdAdd:
          case X86Inst::kIdAnd:
          case X86Inst::kIdRol:
          case X86Inst::kIdRor:
          case X86Inst::kIdSar:
          case X86Inst::kIdShl:
          case X86Inst::kIdShr:
          case X86Inst::kIdSub:
          case X86Inst::kIdXor: {
            // Updates [E|R]FLAGS without changing the content.
            if (reg.getSize() != 4 || reg.getSize() >= workRegSize) {
              if (imm.getUInt64() == 0)
                singleRegCase = X86Inst::kSingleRegRO;
            }
            break;
          }
        }
      }

      switch (singleRegCase) {
        case X86Inst::kSingleRegNone:
          break;
        case X86Inst::kSingleRegRO:
          ib[0]->makeReadOnly();
          opInfo = X86OpInfo::op_r;
          break;
        case X86Inst::kSingleRegWO:
          ib[0]->makeWriteOnly();
          opInfo = X86OpInfo::op_w_all;
          break;
      }
    }

    controlType = commonData.getControlType();
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RACFGBuilder - OnCall]
// ============================================================================

Error X86RACFGBuilder::onCall(CCFuncCall* call, RAInstBuilder& ib) noexcept {
  uint32_t argCount = call->getArgCount();
  uint32_t retCount = call->getRetCount();
  const FuncDetail& fd = call->getDetail();

  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
      if (!fd.hasArg(argIndex + argHi))
        continue;

      const FuncValue& arg = fd.getArg(argIndex + argHi);
      const Operand& op = call->getArg(argIndex + argHi);

      if (op.isNone())
        continue;

      if (op.isReg()) {
        const X86Reg& reg = op.as<X86Reg>();
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.getId()), &workReg));

        if (arg.isReg()) {
          uint32_t regGroup = workReg->getGroup();
          uint32_t argGroup = X86Reg::groupOf(arg.getRegType());

          if (regGroup == argGroup) {
            ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, 0, arg.getRegId(), 0, X86Gp::kIdBad, 0));
          }
          else {
            // TODO:
            ASMJIT_ASSERT(!"IMPLEMENTED");
          }
        }
        else {
          ASMJIT_PROPAGATE(moveRegToStackArg(call, arg, op.as<Reg>()));
        }
      }
      else if (op.isImm()) {
        if (arg.isReg()) {
          Reg reg;
          ASMJIT_PROPAGATE(moveImmToRegArg(call, arg, op.as<Imm>(), &reg));

          RAWorkReg* workReg;
          ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.getId()), &workReg));
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, 0, arg.getRegId(), 0, X86Gp::kIdBad, 0));
        }
        else {
          ASMJIT_PROPAGATE(moveImmToStackArg(call, arg, op.as<Imm>()));
        }
      }
      else {
        return DebugUtils::errored(kErrorInvalidState);
      }
    }
  }

  for (uint32_t retIndex = 0; retIndex < retCount; retIndex++) {
    const FuncValue& ret = fd.getRet(retIndex);
    const Operand& op = call->getRet(retIndex);

    if (op.isReg()) {
      const X86Reg& reg = op.as<X86Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.getId()), &workReg));

      if (ret.isReg()) {
        uint32_t regGroup = workReg->getGroup();
        uint32_t retGroup = X86Reg::groupOf(ret.getRegType());

        if (regGroup == retGroup) {
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kOut | RATiedReg::kWrite, 0, X86Gp::kIdBad, 0, ret.getRegId(), 0));
        }
        else {
          // TODO:
          ASMJIT_ASSERT(!"IMPLEMENTED");
        }
      }
      else {
        return DebugUtils::errored(kErrorInvalidState);
      }
    }
  }

  // Setup clobbered registers.
  ib._clobbered[0] = IntUtils::lsbMask<uint32_t>(_pass->_physRegCount[0]) & ~fd.getPreservedRegs(0);
  ib._clobbered[1] = IntUtils::lsbMask<uint32_t>(_pass->_physRegCount[1]) & ~fd.getPreservedRegs(1);
  ib._clobbered[2] = IntUtils::lsbMask<uint32_t>(_pass->_physRegCount[2]) & ~fd.getPreservedRegs(2);
  ib._clobbered[3] = IntUtils::lsbMask<uint32_t>(_pass->_physRegCount[3]) & ~fd.getPreservedRegs(3);

  // This block has function call(s).
  _pass->getFunc()->getFrame().updateCallStackSize(fd.getArgStackSize());
  _curBlock->addFlags(RABlock::kFlagHasFuncCalls);

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RACFGBuilder - MoveImmToRegArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToRegArg(CCFuncCall* call, const FuncValue& arg, const Imm& imm_, Reg* out) noexcept {
  ASMJIT_ASSERT(arg.isReg());

  Imm imm(imm_);
  switch (arg.getTypeId()) {
    case Type::kIdI8: imm.signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm.zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm.signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm.zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
MovU32:
      imm.zeroExtend32Bits();
      ASMJIT_PROPAGATE(cc()->_newReg(*out, Type::kIdU32, nullptr));

MovAny:
      {
        CBInst* inst;
        uint32_t physIds[] = { arg.getRegId(), Reg::kIdBad };
        ASMJIT_PROPAGATE(newInst(&inst, X86Inst::kIdMov, X86OpInfo::op_w, physIds, *out, imm));

        cc()->addBefore(inst, call);
        return kErrorOk;
      }

    case Type::kIdI64:
    case Type::kIdU64:
      // Prefer smaller code, moving to GPD automatically zero extends in 64-bit mode.
      if (imm.isUInt32())
        goto MovU32;

      ASMJIT_PROPAGATE(cc()->_newReg(*out, Type::kIdU64, nullptr));
      goto MovAny;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }
}

// ============================================================================
// [asmjit::X86RACFGBuilder - MoveImmToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToStackArg(CCFuncCall* call, const FuncValue& arg, const Imm& imm_) noexcept {
  ASMJIT_ASSERT(arg.isStack());

  X86Mem mem = x86::ptr(_pass->_sp.as<X86Gp>(), arg.getStackOffset());
  Imm imm[2];

  mem.setSize(4);
  imm[0] = imm_;

  uint32_t nMovs = 0;
  static const uint32_t noPhysIds[] = { Reg::kIdBad, Reg::kIdBad };

  // One stack entry has the same size as the native register size. That means
  // that if we want to move a 32-bit integer on the stack in 64-bit mode, we
  // need to extend it to a 64-bit integer first. In 32-bit mode, pushing a
  // 64-bit on stack is done in two steps by pushing low and high parts
  // separately.
  switch (arg.getTypeId()) {
    case Type::kIdI8: imm[0].signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm[0].zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm[0].signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm[0].zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdF32:
MovU32:
      imm[0].zeroExtend32Bits();
      nMovs = 1;
      break;

    case Type::kIdI64:
    case Type::kIdU64:
    case Type::kIdF64:
    case Type::kIdMmx32:
    case Type::kIdMmx64:
      if (_is64Bit && imm[0].isInt32()) {
        mem.setSize(8);
        nMovs = 1;
        break;
      }

      imm[1].setU32(imm[0].getUInt32Hi());
      imm[0].zeroExtend32Bits();
      nMovs = 2;
      break;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  for (uint32_t i = 0; i < nMovs; i++) {
    CBInst* inst;
    ASMJIT_PROPAGATE(newInst(&inst, X86Inst::kIdMov, X86OpInfo::op_w, noPhysIds, mem, imm[i]));
    cc()->addBefore(inst, call);
    mem.addOffsetLo32(int32_t(mem.getSize()));
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RACFGBuilder - MoveRegToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveRegToStackArg(CCFuncCall* call, const FuncValue& arg, const Reg& reg) noexcept {
  ASMJIT_ASSERT(arg.isStack());

  return kErrorOk;
  /*
  X86Mem mem = x86::ptr(_pass->_sp.as<X86Gp>(), arg.getStackOffset());
  X86Reg r0, r1;

  uint32_t gpSize = cc()->getGpSize();
  uint32_t instId = 0;

  uint32_t dstTypeId = arg.getTypeId();
  // uint32_t srcTypeId = reg.;


  switch (dstTypeId) {
    case Type::kIdI64:
    case Type::kIdU64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGp8(srcTypeId)) {
        r1.setX86RegT<X86Reg::kRegGpbLo>(srcPhysId);

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI8) ? X86Inst::kIdMovsx : X86Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGpw(srcTypeId)) {
        r1.setX86RegT<X86Reg::kRegGpw>(srcPhysId);

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI16) ? X86Inst::kIdMovsx : X86Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (GP).
      if (Type::isGpd(srcTypeId)) {
        r1.setX86RegT<X86Reg::kRegGpd>(srcPhysId);

        instId = X86Inst::kIdMovsxd;
        if (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI32)
          goto _ExtendMovGpXQ;
        else
          goto _ZeroExtendGpDQ;
      }

      // Move QWORD (GP).
      if (Type::isGpq(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdI16:
    case Type::kIdU16:
      // DWORD <- WORD (Zero|Sign Extend).
      if (Type::isGpw(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setX86RegT<X86Reg::kRegGpw>(srcPhysId);
        instId = isDstSigned && isSrcSigned ? X86Inst::kIdMovsx : X86Inst::kIdMovzx;
        goto _ExtendMovGpD;
      }

      // DWORD <- BYTE (Zero|Sign Extend).
      if (Type::isGpb(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setX86RegT<X86Reg::kRegGpbLo>(srcPhysId);
        instId = isDstSigned && isSrcSigned ? X86Inst::kIdMovsx : X86Inst::kIdMovzx;
        goto _ExtendMovGpD;
      }
      ASMJIT_FALLTHROUGH;

    case Type::kIdI8:
    case Type::kIdU8:
      if (Type::isInt(srcTypeId)) goto MovGpD;
      if (Type::isMmx(srcTypeId)) goto MovMmD;
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdMmx32:
    case Type::kIdMmx64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGpb(srcTypeId)) {
        r1.setX86RegT<X86Reg::kRegGpbLo>(srcPhysId);

        instId = X86Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGpw(srcTypeId)) {
        r1.setX86RegT<X86Reg::kRegGpw>(srcPhysId);

        instId = X86Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      if (Type::isGpd(srcTypeId)) goto _ExtendMovGpDQ;
      if (Type::isGpq(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdF32:
    case Type::kIdF32x1:
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdF64:
    case Type::kIdF64x1:
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    default:
      // TODO: Vector types by stack.
      break;
  }
  return DebugUtils::errored(kErrorInvalidState);

  // Extend+Move Gp.
_ExtendMovGpD:
  m0.setSize(4);
  r0.setX86RegT<X86Reg::kRegGpd>(srcPhysId);

  cc()->emit(instId, r0, r1);
  cc()->emit(X86Inst::kIdMov, m0, r0);
  return kErrorOk;

_ExtendMovGpXQ:
  if (gpSize == 8) {
    m0.setSize(8);
    r0.setX86RegT<X86Reg::kRegGpq>(srcPhysId);

    cc()->emit(instId, r0, r1);
    cc()->emit(X86Inst::kIdMov, m0, r0);
  }
  else {
    m0.setSize(4);
    r0.setX86RegT<X86Reg::kRegGpd>(srcPhysId);

    cc()->emit(instId, r0, r1);

_ExtendMovGpDQ:
    cc()->emit(X86Inst::kIdMov, m0, r0);
    m0.addOffsetLo32(4);
    cc()->emit(X86Inst::kIdAnd, m0, 0);
  }
  return kErrorOk;

_ZeroExtendGpDQ:
  m0.setSize(4);
  r0.setX86RegT<X86Reg::kRegGpd>(srcPhysId);
  goto _ExtendMovGpDQ;

MovGpD:
  m0.setSize(4);
  r0.setX86RegT<X86Reg::kRegGpd>(srcPhysId);
  return cc()->emit(X86Inst::kIdMov, m0, r0);

MovGpQ:
  m0.setSize(8);
  r0.setX86RegT<X86Reg::kRegGpq>(srcPhysId);
  return cc()->emit(X86Inst::kIdMov, m0, r0);

MovMmD:
  m0.setSize(4);
  r0.setX86RegT<X86Reg::kRegMm>(srcPhysId);
  return cc()->emit(X86Inst::kIdMovd, m0, r0);

MovMmQ:
  m0.setSize(8);
  r0.setX86RegT<X86Reg::kRegMm>(srcPhysId);
  return cc()->emit(X86Inst::kIdMovq, m0, r0);

MovXmmD:
  m0.setSize(4);
  r0.setX86RegT<X86Reg::kRegXmm>(srcPhysId);
  return cc()->emit(X86Inst::kIdMovss, m0, r0);

MovXmmQ:
  m0.setSize(8);
  r0.setX86RegT<X86Reg::kRegXmm>(srcPhysId);
  return cc()->emit(X86Inst::kIdMovlps, m0, r0);
  */
}

// ============================================================================
// [asmjit::X86RACFGBuilder - OnReg]
// ============================================================================

Error X86RACFGBuilder::onRet(CCFuncRet* funcRet, RAInstBuilder& ib) noexcept {
  const FuncDetail& funcDetail = _pass->getFunc()->getDetail();
  const Operand* opArray = funcRet->getOpArray();
  uint32_t opCount = funcRet->getOpCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = opArray[i];
    if (op.isNone()) continue;

    const FuncValue& ret = funcDetail.getRet(i);
    if (ASMJIT_UNLIKELY(!ret.isReg()))
      return DebugUtils::errored(kErrorInvalidState);

    if (op.isReg()) {
      // Register return value.
      const X86Reg& reg = op.as<X86Reg>();
      uint32_t vIndex = Operand::unpackId(reg.getId());

      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->getGroup();
        uint32_t allocable = _pass->_availableRegs[group];
        ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, ret.getRegId(), 0, Reg::kIdBad, 0));
      }
    }
    else {
      return DebugUtils::errored(kErrorInvalidState);
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RAPass - Construction / Destruction]
// ============================================================================

X86RAPass::X86RAPass() noexcept
  : RAPass(),
    _avxEnabled(false) {}
X86RAPass::~X86RAPass() noexcept {}

// ============================================================================
// [asmjit::X86RAPass - OnInit / OnDone]
// ============================================================================

void X86RAPass::onInit() noexcept {
  uint32_t archType = cc()->getArchType();
  uint32_t baseRegCount = archType == ArchInfo::kTypeX86 ? 8U : 16U;

  _archTraits[X86Reg::kGroupGp] |= RAArchTraits::kHasSwap;

  _physRegCount.set(X86Reg::kGroupGp , baseRegCount);
  _physRegCount.set(X86Reg::kGroupVec, baseRegCount);
  _physRegCount.set(X86Reg::kGroupMm , 8);
  _physRegCount.set(X86Reg::kGroupK  , 8);
  _buildPhysIndex();

  _availableRegCount = _physRegCount;
  _availableRegs[X86Reg::kGroupGp ] = IntUtils::lsbMask<uint32_t>(_physRegCount.get(X86Reg::kGroupGp ));
  _availableRegs[X86Reg::kGroupVec] = IntUtils::lsbMask<uint32_t>(_physRegCount.get(X86Reg::kGroupVec));
  _availableRegs[X86Reg::kGroupMm ] = IntUtils::lsbMask<uint32_t>(_physRegCount.get(X86Reg::kGroupMm ));
  _availableRegs[X86Reg::kGroupK  ] = IntUtils::lsbMask<uint32_t>(_physRegCount.get(X86Reg::kGroupK  ));

  // The architecture specific setup makes implicitly all registers available. So
  // make unavailable all registers that are special and cannot be used in general.
  bool hasFP = _func->getFrame().hasPreservedFP();

  makeUnavailable(X86Reg::kGroupGp, X86Gp::kIdSp);            // ESP|RSP used as a stack-pointer (SP).
  if (hasFP) makeUnavailable(X86Reg::kGroupGp, X86Gp::kIdBp); // EBP|RBP used as a frame-pointer (FP).

  _sp = cc()->zsp();
  _fp = cc()->zbp();
  _avxEnabled = _func->getFrame().isAvxEnabled();
}

void X86RAPass::onDone() noexcept {}

// ============================================================================
// [asmjit::X86RAPass - BuildCFG]
// ============================================================================

Error X86RAPass::buildCFG() noexcept {
  return X86RACFGBuilder(this).run();
}

// ============================================================================
// [asmjit::X86RAPass - OnEmit]
// ============================================================================

Error X86RAPass::onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = getWorkReg(workId);
  Reg dst(wReg->getInfo().getSignature(), dstPhysId);
  Reg src(wReg->getInfo().getSignature(), srcPhysId);

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerOptions & Logger::kOptionAnnotate) {
    _tmpString.setFormat("<MOVE> %s", getWorkReg(workId)->getName());
    comment = _tmpString.getData();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<X86Emitter>(), dst, src, wReg->getTypeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  RAWorkReg* waReg = getWorkReg(aWorkId);
  RAWorkReg* wbReg = getWorkReg(bWorkId);

  bool is64Bit = std::max(waReg->getTypeId(), wbReg->getTypeId()) >= Type::kIdI64;
  uint32_t sign = is64Bit ? uint32_t(X86RegTraits<X86Reg::kRegGpq>::kSignature)
                          : uint32_t(X86RegTraits<X86Reg::kRegGpd>::kSignature);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerOptions & Logger::kOptionAnnotate) {
    _tmpString.setFormat("<SWAP> %s, %s", waReg->getName(), wbReg->getName());
    cc()->setInlineComment(_tmpString.getData());
  }
  #endif

  return cc()->emit(X86Inst::kIdXchg, X86Reg(sign, aPhysId), X86Reg(sign, bPhysId));
}

Error X86RAPass::onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  RAWorkReg* wReg = getWorkReg(workId);
  Reg dstReg(wReg->getInfo().getSignature(), dstPhysId);
  Mem srcMem(workRegAsMem(wReg));

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerOptions & Logger::kOptionAnnotate) {
    _tmpString.setFormat("<LOAD> %s", getWorkReg(workId)->getName());
    comment = _tmpString.getData();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<X86Emitter>(), dstReg, srcMem, wReg->getTypeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = getWorkReg(workId);
  Mem dstMem(workRegAsMem(wReg));
  Reg srcReg(wReg->getInfo().getSignature(), srcPhysId);

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerOptions & Logger::kOptionAnnotate) {
    _tmpString.setFormat("<SAVE> %s", getWorkReg(workId)->getName());
    comment = _tmpString.getData();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<X86Emitter>(), dstMem, srcReg, wReg->getTypeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitJump(const Label& label) noexcept {
  return cc()->jmp(label);
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER

























































































#if 0
ASMJIT_FORCEINLINE void X86CallAlloc::allocImmsOnStack() {
  CCFuncCall* node = getNode();
  FuncDetail& fd = node->getDetail();

  uint32_t argCount = fd.getArgCount();
  Operand_* args = node->_args;

  for (uint32_t i = 0; i < argCount; i++) {
    Operand_& op = args[i];
    if (!op.isImm()) continue;

    const Imm& imm = static_cast<const Imm&>(op);
    const FuncDetail::Value& arg = fd.getArg(i);
    uint32_t varType = arg.getTypeId();

    if (arg.isReg()) {
      _context->emitImmToReg(varType, arg.getRegId(), &imm);
    }
    else {
      X86Mem dst = x86::ptr(_context->_zsp, -int(_context->getGpSize()) + arg.getStackOffset());
      _context->emitImmToStack(varType, &dst, &imm);
    }
  }
}

template<int C>
ASMJIT_FORCEINLINE void X86CallAlloc::duplicate() {
  TiedReg* tiedRegs = getTiedRegsByGroup(C);
  uint32_t tiedCount = getTiedCountByGroup(C);

  for (uint32_t i = 0; i < tiedCount; i++) {
    TiedReg* tied = &tiedRegs[i];
    if ((tied->flags & TiedReg::kRReg) == 0) continue;

    uint32_t inRegs = tied->inRegs;
    if (!inRegs) continue;

    VirtReg* vreg = tied->vreg;
    uint32_t physId = vreg->getPhysId();

    ASMJIT_ASSERT(physId != Reg::kIdBad);

    inRegs &= ~IntUtils::mask(physId);
    if (!inRegs) continue;

    for (uint32_t dupIndex = 0; inRegs != 0; dupIndex++, inRegs >>= 1) {
      if (inRegs & 0x1) {
        _context->emitMove(vreg, dupIndex, physId, "Duplicate");
        _context->_clobberedRegs.or_(C, IntUtils::mask(dupIndex));
      }
    }
  }
}

// ============================================================================
// [asmjit::X86CallAlloc - Ret]
// ============================================================================

ASMJIT_FORCEINLINE void X86CallAlloc::ret() {
  CCFuncCall* node = getNode();
  FuncDetail& fd = node->getDetail();
  Operand_* rets = node->_ret;

  for (uint32_t i = 0; i < 2; i++) {
    const FuncDetail::Value& ret = fd.getRet(i);
    Operand_* op = &rets[i];

    if (!ret.isReg() || !op->isVirtReg())
      continue;

    VirtReg* vreg = _cc->getVirtRegById(op->getId());
    uint32_t regId = ret.getRegId();

    switch (vreg->getGroup()) {
      case X86Reg::kGroupGp:
        _context->unuse<X86Reg::kGroupGp>(vreg);
        _context->attach<X86Reg::kGroupGp>(vreg, regId, true);
        break;

      case X86Reg::kGroupMm:
        _context->unuse<X86Reg::kGroupMm>(vreg);
        _context->attach<X86Reg::kGroupMm>(vreg, regId, true);
        break;

      case X86Reg::kGroupVec:
        if (X86Reg::groupOf(ret.getRegType()) == X86Reg::kGroupVec) {
          _context->unuse<X86Reg::kGroupVec>(vreg);
          _context->attach<X86Reg::kGroupVec>(vreg, regId, true);
        }
        else {
          uint32_t elementId = Type::baseOf(vreg->getTypeId());
          uint32_t size = (elementId == Type::kIdF32) ? 4 : 8;

          X86Mem m = _context->getVarMem(vreg);
          m.setSize(size);

          _context->unuse<X86Reg::kGroupVec>(vreg, VirtReg::kStateMem);
          _cc->fstp(m);
        }
        break;
    }
  }
}

// ============================================================================
// [asmjit::X86RAPass - Translate - Ret]
// ============================================================================

static Error X86RAPass_translateRet(X86RAPass* self, CCFuncRet* rNode, CBLabel* exitTarget) {
  X86Compiler* cc = self->cc();
  CBNode* node = rNode->getNext();

  // 32-bit mode requires to push floating point return value(s), handle it
  // here as it's a special case.
  X86RAData* raData = rNode->getPassData<X86RAData>();
  if (raData) {
    TiedReg* tiedRegs = raData->tiedRegs;
    uint32_t tiedTotal = raData->tiedTotal;

    for (uint32_t i = 0; i < tiedTotal; i++) {
      TiedReg* tied = &tiedRegs[i];
      if (tied->flags & (TiedReg::kX86Fld4 | TiedReg::kX86Fld8)) {
        VirtReg* vreg = tied->vreg;
        X86Mem m(self->getVarMem(vreg));

        uint32_t elementId = Type::baseOf(vreg->getTypeId());
        m.setSize(elementId == Type::kIdF32 ? 4 :
                  elementId == Type::kIdF64 ? 8 :
                  (tied->flags & TiedReg::kX86Fld4) ? 4 : 8);

        cc->fld(m);
      }
    }
  }

  // Decide whether to `jmp` or not in case we are next to the return label.
  while (node) {
    switch (node->getType()) {
      // If we have found an exit label we just return, there is no need to
      // emit jump to that.
      case CBNode::kNodeLabel:
        if (static_cast<CBLabel*>(node) == exitTarget)
          return kErrorOk;
        goto _EmitRet;

      case CBNode::kNodeData:
      case CBNode::kNodeInst:
      case CBNode::kNodeFuncCall:
      case CBNode::kNodeFuncExit:
        goto _EmitRet;

      // Continue iterating.
      case CBNode::kNodeComment:
      case CBNode::kNodeAlign:
      case CBNode::kNodeHint:
        break;

      // Invalid node to be here.
      case CBNode::kNodeFunc:
        return DebugUtils::errored(kErrorInvalidState);

      // We can't go forward from here.
      case CBNode::kNodeSentinel:
        return kErrorOk;
    }

    node = node->getNext();
  }

_EmitRet:
  {
    cc->_setCursor(rNode);
    cc->jmp(exitTarget->getLabel());
  }
  return kErrorOk;
}
#endif
